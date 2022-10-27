#include "material_shader_assembler.h"
#include "material_shader.h"
#include "shader_program.h"
#include "pathos/util/file_system.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#define MATERIAL_FOLDER            "shaders/materials/"
#define MATERIAL_TEMPLATE_FILENAME "_template.glsl"

// Tokens in the template which are filled by the material assembler or material shaders
#define NEED_HEADER                "$NEED"
#define NEED_SHADERSTAGE           "SHADERSTAGE"
#define NEED_SHADINGMODEL          "SHADINGMODEL"
#define NEED_OUTPUTWORLDNORMAL     "OUTPUTWORLDNORMAL"
#define NEED_UBO                   "UBO_Material"
#define NEED_TEXTUREPARAMETERS     "TEXTURE_PARAMETERS"
#define NEED_VPO                   "getVertexPositionOffset"
#define NEED_MATERIALATTRS         "getMaterialAttributes"
#define NEED_GETSCENECOLOR         "getSceneColor"

// Macros that are used by material shaders
#define PARAMETER_CONSTANT         "PARAMETER_CONSTANT"
#define PARAMETER_TEXTURE          "PARAMETER_TEXTURE"

namespace pathos {

	void MaterialTemplate::updatePlaceholderIx() {
		// Find $NEED lines
		MaterialTemplate& MT = *this;
		const std::string NEED(NEED_HEADER);
		const int32 totalLines = (int32)MT.sourceLines.size();
		for (int32 lineIx = 0; lineIx < totalLines; ++lineIx) {
			const std::string& line = MT.sourceLines[lineIx];
			if (line.find(NEED) == 0) {
				std::string header = line.substr(NEED.size() + 1);
				if (header == NEED_SHADERSTAGE) {
					MT.lineIx_shaderstage = lineIx;
				} else if (header == NEED_SHADINGMODEL) {
					MT.lineIx_shadingmodel = lineIx;
				} else if (header == NEED_OUTPUTWORLDNORMAL) {
					MT.lineIx_outputworldnormal = lineIx;
				} else if (header == NEED_UBO) {
					MT.lineIx_ubo = lineIx;
				} else if (header == NEED_TEXTUREPARAMETERS) {
					MT.lineIx_textureParams = lineIx;
				} else if (header == NEED_VPO) {
					MT.lineIx_getVPO = lineIx;
				} else if (header == NEED_MATERIALATTRS) {
					MT.lineIx_getMaterialAttrs = lineIx;
				} else if (header == NEED_GETSCENECOLOR) {
					MT.lineIx_getSceneColor = lineIx;
				}
			}
		}
		CHECK(MT.checkPlaceholders());
	}

	void splitNewlines(std::vector<std::string>& inoutLines) {
		std::vector<std::string> tempLines = inoutLines;
		inoutLines.clear();
		// Split lines that contain multiple newlines.
		for (const std::string& line : tempLines) {
			size_t offset = 0;
			size_t len = line.size();
			while (true) {
				size_t newline = line.find_first_of('\n', offset);
				// Exclude '\n' at end.
				std::string tok = line.substr(offset, newline - offset);
				inoutLines.push_back(tok);
				if (newline == std::string::npos) {
					break;
				}
				offset = newline + 1;
			}
		}
	}

}

namespace pathos {

	MaterialShaderAssembler& MaterialShaderAssembler::get() {
		static MaterialShaderAssembler instance;
		return instance;
	}

	MaterialShaderAssembler::~MaterialShaderAssembler() {
		if (materialTemplate != nullptr) {
			delete materialTemplate;
			materialTemplate = nullptr;
		}
	}

	void MaterialShaderAssembler::initializeMaterialShaders() {
		loadMaterialTemplate();
		parseAllMaterialShaders();
	}

	MaterialShader* MaterialShaderAssembler::findMaterialShaderByHash(uint32 materialNameHash) {
		auto it = materialShaderMap.find(materialNameHash);
		if (it == materialShaderMap.end()) {
			return nullptr;
		}
		return it->second;
	}

	MaterialShader* MaterialShaderAssembler::findMaterialShaderByName(const char* materialName) {
		for (auto it = materialShaderMap.begin(); it != materialShaderMap.end(); ++it) {
			MaterialShader* ms = it->second;
			if (ms->materialName == materialName) {
				return ms;
			}
		}
		return nullptr;
	}

	void MaterialShaderAssembler::loadMaterialTemplate() {
		if (bTemplateLoaded) {
			return;
		}

		std::string templatePathRel = MATERIAL_FOLDER;
		templatePathRel += MATERIAL_TEMPLATE_FILENAME;

		materialTemplate = new MaterialTemplate;
		MaterialTemplate& MT = *materialTemplate;

		// #todo-material-assembler: Parse includes when loading the template.
		// Then how to parse includes in material shaders?
		std::vector<std::string> emptyDefines;
		std::vector<std::string> includeHistory;
		ShaderStage::loadSourceInternal(templatePathRel, emptyDefines, 0, includeHistory, MT.sourceLines);
		splitNewlines(MT.sourceLines);

		// Find $NEED lines
		const std::string NEED(NEED_HEADER);
		MT.updatePlaceholderIx();

		bTemplateLoaded = true;
	}

	void MaterialShaderAssembler::parseAllMaterialShaders() {
		CHECKF(bTemplateLoaded, "Material template is not ready");

		std::string shaderDir = ResourceFinder::get().find(MATERIAL_FOLDER);
		shaderDir = pathos::getAbsolutePath(shaderDir.c_str());
		std::vector<std::string> files;
		pathos::enumerateFiles(shaderDir.c_str(), files);
		for (const std::string& filename : files) {
			if (filename == MATERIAL_TEMPLATE_FILENAME) {
				continue;
			}
			
			std::string materialPath = shaderDir + filename;
			MaterialShader* material = generateMaterialShader(materialPath.c_str(), filename.c_str());
			CHECK(material != nullptr);
		}
	}

	MaterialShader* MaterialShaderAssembler::generateMaterialShader(const char* fullpath, const char* filename) {
		struct TextureParameterDesc {
			uint32 lineIx;
			uint32 binding;
			std::string samplerType;
			std::string name;

			std::string toString() const {
				char buf[256];
				sprintf_s(buf, "layout (binding = %u) uniform %s %s;",
					binding, samplerType.c_str(), name.c_str());
				return std::string(buf);
			}
		};
		struct ConstantParameterDesc {
			uint32 lineIx;
			std::string datatype;
			std::string name;
			uint32 numElements;
			EMaterialParameterDataType datatypeEnum = EMaterialParameterDataType::Float;

			ConstantParameterDesc(uint32 inLineIx, const std::string& inDatatype, const std::string& inName)
				: lineIx(inLineIx), datatype(inDatatype), name(inName)
			{
				if (datatype == "float" || datatype == "int" || datatype == "uint" || datatype == "bool") {
					numElements = 1;
				} else if (datatype == "vec2" || datatype == "ivec2" || datatype == "uvec2" || datatype == "bvec2") {
					numElements = 2;
				} else if (datatype == "vec3" || datatype == "ivec3" || datatype == "uvec3" || datatype == "bvec3") {
					numElements = 3;
				} else if (datatype == "vec4" || datatype == "ivec4" || datatype == "uvec4" || datatype == "bvec4") {
					numElements = 4;
				} else {
					numElements = 0;
					CHECK_NO_ENTRY();
				}

				if (datatype == "float" || datatype == "vec2" || datatype == "vec3" || datatype == "vec4") {
					datatypeEnum = EMaterialParameterDataType::Float;
				} else if (datatype == "int" || datatype == "ivec2" || datatype == "ivec3" || datatype == "ivec4") {
					datatypeEnum = EMaterialParameterDataType::Int;
				} else if (datatype == "uint" || datatype == "uvec2" || datatype == "uvec3" || datatype == "uvec4") {
					datatypeEnum = EMaterialParameterDataType::Uint;
				} else if (datatype == "bool" || datatype == "bvec2" || datatype == "bvec3" || datatype == "bvec4") {
					datatypeEnum = EMaterialParameterDataType::Bool;
				}
			}

			std::string toString() const {
				char buf[256];
				sprintf_s(buf, "%s %s;", datatype.c_str(), name.c_str());
				return std::string(buf);
			}
		}; // End of ConstantParameterDesc

		std::fstream fileStream;
		fileStream.open(fullpath, std::ios::in);
		if (fileStream.is_open() == false) {
			LOG(LogError, "[Material] Failed to open: %s", fullpath);
			return nullptr;
		}

		std::string materialName = filename;
		materialName = materialName.substr(0, materialName.find_first_of('.'));

		const uint32 materialNameHash = COMPILE_TIME_CRC32_STR(materialName.c_str());
		CHECKF(materialShaderMap.find(materialNameHash) == materialShaderMap.end(), "Material name conflict");

		// Gather original lines
		std::vector<std::string> materialLines;
		while (fileStream) {
			std::string line;
			std::getline(fileStream, line);
			materialLines.emplace_back(line);
		}

		// Parse parameters
		const int32 totalMaterialLines = (int32)materialLines.size();
		std::vector<TextureParameterDesc> textureParams;
		std::vector<ConstantParameterDesc> constParams;
		for (int32 lineIx = 0; lineIx < totalMaterialLines; ++lineIx) {
			const std::string& line = materialLines[lineIx];
			if (line.find(PARAMETER_TEXTURE) == 0) {
				uint32 binding = 0xffffffff;
				char samplerType[16];
				char textureName[64];
				int ret = sscanf_s(line.c_str(),
					"PARAMETER_TEXTURE(%u, %[^,], %[^)])",
					&binding,
					samplerType, (unsigned)_countof(samplerType),
					textureName, (unsigned)_countof(textureName));
				CHECK(ret == 3);

				textureParams.push_back(
					TextureParameterDesc{ (uint32)lineIx, binding, samplerType, textureName });
			} else if (line.find(PARAMETER_CONSTANT) == 0) {
				char datatype[16];
				char name[64];
				// #todo-material-assembler: Support default values?
				// It would be quite useful but another parsing mess :/
				int ret = sscanf_s(line.c_str(),
					"PARAMETER_CONSTANT(%[^,], %[^)])",
					datatype, (unsigned)_countof(datatype),
					name, (unsigned)_countof(name));
				CHECK(ret == 2);

				constParams.push_back(
					ConstantParameterDesc((uint32)lineIx, datatype, name));
			}
		}

		// Filled later
		std::vector<MaterialConstantParameter> materialConstParameters;
		std::vector<MaterialTextureParameter> materialTextureParameters;

		// Find definitions
		int32 materialShadingModelIx = -1;
		int32 materialVPOBeginIx = -1; // inclusive
		int32 materialVPOEndIx = -1; // inclusive
		int32 materialAttrBeginIx = -1; // inclusive
		int32 materialAttrEndIx = -1; // inclusive
		int32 getSceneColorBeginIx = -1; // inclusive
		int32 getSceneColorEndIx = -1; // inclusive
		bool bTrivialDepthOnlyPass = true;
		bool bOutputWorldNormal = false;
		for (int32 lineIx = 0; lineIx < totalMaterialLines; ++lineIx) {
			const std::string& line = materialLines[lineIx];
			if (0 == line.find("#define SHADINGMODEL")) {
				materialShadingModelIx = lineIx;
			} else if (0 == line.find("#define NONTRIVIALDEPTH")) {
				bTrivialDepthOnlyPass = false;
			} else if (0 == line.find("#define OUTPUTWORLDNORMAL")) {
				bOutputWorldNormal = true;
			} else if (0 == line.find("VPO_BEGIN")) {
				materialVPOBeginIx = lineIx + 1;
			} else if (0 == line.find("VPO_END")) {
				materialVPOEndIx = lineIx - 1;
			} else if (0 == line.find("ATTR_BEGIN")) {
				materialAttrBeginIx = lineIx + 1;
			} else if (0 == line.find("ATTR_END")) {
				materialAttrEndIx = lineIx - 1;
			} else if (0 == line.find("FORWARDSHADING_BEGIN")) {
				getSceneColorBeginIx = lineIx + 1;
			} else if (0 == line.find("FORWARDSHADING_END")) {
				getSceneColorEndIx = lineIx - 1;
			}
		}
		const bool bMaterialWellDefined =
			(materialShadingModelIx != -1)
			&& (materialVPOBeginIx != -1)
			&& (materialVPOEndIx != -1)
			&& (materialVPOBeginIx < materialVPOEndIx)
			&& (materialAttrBeginIx != -1)
			&& (materialAttrEndIx != -1)
			&& (materialAttrBeginIx < materialAttrEndIx);
		CHECK(bMaterialWellDefined);

		MaterialTemplate MT = materialTemplate->makeClone();
		MT.replaceShadingModel(materialLines[materialShadingModelIx]);

		if (bOutputWorldNormal) {
			MT.replaceOutputWorldNormal("#define OUTPUTWORLDNORMAL 1");
		} else {
			MT.replaceOutputWorldNormal("");
		}

		// Construct material uniform buffer.
		uint32 uboTotalElements = 0; // 1 element = 4 bytes
		if (constParams.size() == 0) {
			MT.replaceUBO("");
		} else {
			std::sort(constParams.begin(), constParams.end(),
				[](ConstantParameterDesc& A, ConstantParameterDesc& B) -> bool {
					return A.numElements > B.numElements;
				}
			);
			std::string ubo = "layout (std140, binding = UBO_BINDING_MATERIAL) uniform UBO_Material {\n";
			int32 uboPaddingId = 0;
			int32 numScalars = 0;
			uint32 uboCurrentOffset = 0;
			// #todo-material-assembler: More compact packing like (vec3 + float) or (vec2 + vec2).
			for (ConstantParameterDesc& desc : constParams) {
				ubo += '\t';
				ubo += desc.toString();

				MaterialConstantParameter param;
				param.name = desc.name;
				param.datatype = desc.datatypeEnum;
				param.numElements = desc.numElements;
				param.uvalue[0] = param.uvalue[1] = param.uvalue[2] = param.uvalue[3] = 0;
				param.offset = uboCurrentOffset;
				materialConstParameters.emplace_back(param);

				if (desc.numElements == 4) {
					ubo += '\n';
					uboTotalElements += 4;
					uboCurrentOffset += 16;
				} else if (desc.numElements == 3) {
					ubo += "\t\tfloat _unused";
					ubo += std::to_string(uboPaddingId++);
					ubo += ";\n";
					uboTotalElements += 4;
					uboCurrentOffset += 16;
				} else if (desc.numElements == 2) {
					ubo += "\t\tvec2 _unused";
					ubo += std::to_string(uboPaddingId++);
					ubo += ";\n";
					uboTotalElements += 4;
					uboCurrentOffset += 16;
				} else if (desc.numElements == 1) {
					ubo += '\n';
					uboTotalElements += 1;
					uboCurrentOffset += 4;
				} else {
					CHECK_NO_ENTRY();
				}
				numScalars += (desc.numElements == 1) ? 1 : 0;
			}
			switch (numScalars) {
			case 1:
				ubo += "\tvec3 _unused";
				ubo += std::to_string(uboPaddingId++);
				ubo += ";\n";
				uboTotalElements += 3;
				break;
			case 2:
				ubo += "\tvec2 _unused";
				ubo += std::to_string(uboPaddingId++);
				ubo += ";\n";
				uboTotalElements += 2;
				break;
			case 3:
				ubo += "\tfloat _unused";
				ubo += std::to_string(uboPaddingId++);
				ubo += ";\n";
				uboTotalElements += 1;
				break;
			default:
				break;
			}
			ubo += "} uboMaterial; // Total " + std::to_string(4 * uboTotalElements) + " bytes\n";
			MT.replaceUBO(ubo);
		}

		if (textureParams.size() == 0) {
			MT.replaceTextureParameters("");
		} else {
			std::string textures;
			for (const TextureParameterDesc& desc : textureParams) {
				textures += desc.toString();
				textures += '\n';

				MaterialTextureParameter param;
				param.name = desc.name;
				param.binding = desc.binding;
				materialTextureParameters.emplace_back(param);
			}
			MT.replaceTextureParameters(textures);
		}

		std::string vpo;
		for (int32 ix = materialVPOBeginIx; ix <= materialVPOEndIx; ++ix) {
			vpo += materialLines[ix];
			vpo += '\n';
		}
		MT.replaceVPO(vpo);

		std::string attrs;
		for (int32 ix = materialAttrBeginIx; ix <= materialAttrEndIx; ++ix) {
			attrs += materialLines[ix];
			attrs += '\n';
		}
		MT.replaceAttr(attrs);

		// Parse shading model.
		EMaterialShadingModel shadingModel = EMaterialShadingModel::NUM_MODELS;
		{
			char shadingModelStr[64];
			int ret = sscanf_s(materialLines[materialShadingModelIx].c_str(),
				"#define SHADINGMODEL %s",
				shadingModelStr, (unsigned)_countof(shadingModelStr));
			CHECK(ret == 1);

			if (0 == strcmp(shadingModelStr, "MATERIAL_SHADINGMODEL_UNLIT")) {
				shadingModel = EMaterialShadingModel::UNLIT;
			} else if (0 == strcmp(shadingModelStr, "MATERIAL_SHADINGMODEL_DEFAULTLIT")) {
				shadingModel = EMaterialShadingModel::DEFAULTLIT;
			} else if (0 == strcmp(shadingModelStr, "MATERIAL_SHADINGMODEL_TRANSLUCENT")) {
				shadingModel = EMaterialShadingModel::TRANSLUCENT;
			} else {
				char msg[256];
				sprintf_s(msg, "Invalid shading model definition in: %s", shadingModelStr);
				CHECKF(false, msg);
			}
		}

		bool bForwardShading = (shadingModel == EMaterialShadingModel::TRANSLUCENT);
		if (!bForwardShading) {
			MT.replaceGetSceneColor("");
		} else {
			CHECK(getSceneColorBeginIx != -1
				&& getSceneColorEndIx != -1
				&& getSceneColorBeginIx < getSceneColorEndIx);
			std::string getSceneColor;
			for (int32 ix = getSceneColorBeginIx; ix <= getSceneColorEndIx; ++ix) {
				getSceneColor += materialLines[ix];
				getSceneColor += '\n';
			}
			MT.replaceGetSceneColor(getSceneColor);
		}

		// Split newlines again (oneliner long UBOs cause strange shader compilation error).
		splitNewlines(MT.sourceLines);
		MT.updatePlaceholderIx();
		MT.fixupNewlines();

		MaterialShader* shader = new MaterialShader;
		shader->materialName = std::move(materialName);
		shader->shadingModel = shadingModel;
		shader->bTrivialDepthOnlyPass = bTrivialDepthOnlyPass;
		shader->uboTotalBytes = uboTotalElements * 4;
		shader->constantParameters = std::move(materialConstParameters);
		shader->textureParameters = std::move(materialTextureParameters);
		shader->generateShaderProgram(filename, &MT);

		materialShaderMap[materialNameHash] = shader;
		return shader;
	}

}
