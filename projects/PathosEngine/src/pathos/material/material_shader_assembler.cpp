#include "material_shader_assembler.h"
#include "material_shader.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/util/file_system.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#define MATERIAL_FOLDER                   "shaders/materials/"
#define MATERIAL_TEMPLATE_FILENAME        "_template.glsl"

// Tokens in the template which are filled by the material assembler or material shaders.
#define NEED_HEADER                       "$NEED"
#define NEED_SHADERSTAGE                  "SHADERSTAGE"
#define NEED_SHADINGMODEL                 "SHADINGMODEL"
#define NEED_OUTPUTWORLDNORMAL            "OUTPUTWORLDNORMAL"
#define NEED_SKYBOXMATERIAL               "SKYBOXMATERIAL"
#define NEED_TRANSFERDRAWID               "TRANSFER_DRAW_ID"
#define NEED_TRANSFERINSTANCEID           "TRANSFER_INSTANCE_ID"
#define NEED_INDIRECT_DRAW_MODE           "INDIRECT_DRAW_MODE"
#define NEED_UBO                          "UBO_Material"
#define NEED_TEXTUREPARAMETERS            "TEXTURE_PARAMETERS"
#define NEED_VPO                          "getVertexPositionOffset"
#define NEED_MATERIALATTRS                "getMaterialAttributes"
#define NEED_GETSCENECOLOR                "getSceneColor"
#define NEED_EMBEDGLSL                    "embedGlsl"

// Macros that are used by material shaders.
#define PARAMETER_CONSTANT                "PARAMETER_CONSTANT"
#define PARAMETER_TEXTURE                 "PARAMETER_TEXTURE"

// Keywords declared in material shaders.
#define KEYWORD_SHADINGMODEL              "#define SHADINGMODEL"
#define KEYWORD_NONTRIVIALDEPTH           "#define NONTRIVIALDEPTH"
#define KEYWORD_OUTPUTWORLDNORMAL         "#define OUTPUTWORLDNORMAL"
#define KEYWORD_SKYBOXMATERIAL            "#define SKYBOXMATERIAL"
#define KEYWORD_TRANSFER_DRAW_ID          "#define TRANSFER_DRAW_ID"
#define KEYWORD_TRANSFER_INSTANCE_ID      "#define TRANSFER_INSTANCE_ID"
#define KEYWORD_USE_INDIRECT_DRAW         "#define USE_INDIRECT_DRAW"
#define KEYWORD_VPO_BEGIN                 "VPO_BEGIN"
#define KEYWORD_VPO_END                   "VPO_END"
#define KEYWORD_ATTR_BEGIN                "ATTR_BEGIN"
#define KEYWORD_ATTR_END                  "ATTR_END"
#define KEYWORD_FORWARDSHADING_BEGIN      "FORWARDSHADING_BEGIN"
#define KEYWORD_FORWARDSHADING_END        "FORWARDSHADING_END"
#define KEYWORD_EMBED_GLSL_BEGIN          "EMBED_GLSL_BEGIN"
#define KEYWORD_EMBED_GLSL_END            "EMBED_GLSL_END"

// TextureParameterDesc, ConstantParameterDesc, PlaceholderDesc
namespace pathos {
	
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
			: lineIx(inLineIx), datatype(inDatatype), name(inName) {
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
	};

	struct PlaceholderDesc {
		int32 materialShadingModelIx = -1;
		int32 materialVPOBeginIx     = -1; // inclusive
		int32 materialVPOEndIx       = -1; // inclusive
		int32 materialAttrBeginIx    = -1; // inclusive
		int32 materialAttrEndIx      = -1; // inclusive
		int32 getSceneColorBeginIx   = -1; // inclusive
		int32 getSceneColorEndIx     = -1; // inclusive
		int32 embedGlslBeginIx       = -1; // inclusive
		int32 embedGlslEndIx         = -1; // inclusive
		bool bTrivialDepthOnlyPass   = true;
		bool bOutputWorldNormal      = false;
		bool bSkyboxMaterial         = false;
		bool bTransferDrawID         = false;
		bool bTransferInstanceID     = false;
		bool bUseIndirectDraw        = false;
	};

	static void parseMaterialParameters(
		const std::vector<std::string>& materialLines,
		std::vector<TextureParameterDesc>& outTextureParams,
		std::vector<ConstantParameterDesc>& outConstParams)
	{
		const int32 totalMaterialLines = (int32)materialLines.size();
		for (int32 lineIx = 0; lineIx < totalMaterialLines; ++lineIx) {
			const std::string& line = materialLines[lineIx];
			if (0 == line.find(PARAMETER_TEXTURE)) {
				uint32 binding = 0xffffffff;
				char samplerType[16];
				char textureName[64];
				int ret = sscanf_s(line.c_str(),
					"PARAMETER_TEXTURE(%u, %[^,], %[^)])",
					&binding,
					samplerType, (unsigned)_countof(samplerType),
					textureName, (unsigned)_countof(textureName));
				CHECK(ret == 3);

				TextureParameterDesc desc{ (uint32)lineIx, binding, samplerType, textureName };
				outTextureParams.emplace_back(desc);
			} else if (0 == line.find(PARAMETER_CONSTANT)) {
				char datatype[16];
				char name[64];
				// #todo-material-assembler: Support default values?
				// It would be quite useful but another parsing mess :/
				int ret = sscanf_s(line.c_str(),
					"PARAMETER_CONSTANT(%[^,], %[^)])",
					datatype, (unsigned)_countof(datatype),
					name, (unsigned)_countof(name));
				CHECK(ret == 2);

				ConstantParameterDesc desc((uint32)lineIx, datatype, name);
				outConstParams.emplace_back(desc);
			}
		}
	}

	static void scanPlaceholders(const std::vector<std::string>& materialLines, PlaceholderDesc& outDesc) {
		const int32 totalMaterialLines = (int32)materialLines.size();
		for (int32 lineIx = 0; lineIx < totalMaterialLines; ++lineIx) {
			const std::string& line = materialLines[lineIx];
			if (0 == line.find(KEYWORD_SHADINGMODEL))              outDesc.materialShadingModelIx = lineIx;
			else if (0 == line.find(KEYWORD_NONTRIVIALDEPTH))      outDesc.bTrivialDepthOnlyPass  = false;
			else if (0 == line.find(KEYWORD_OUTPUTWORLDNORMAL))    outDesc.bOutputWorldNormal     = true;
			else if (0 == line.find(KEYWORD_SKYBOXMATERIAL))       outDesc.bSkyboxMaterial        = true;
			else if (0 == line.find(KEYWORD_TRANSFER_DRAW_ID))     outDesc.bTransferDrawID        = true;
			else if (0 == line.find(KEYWORD_TRANSFER_INSTANCE_ID)) outDesc.bTransferInstanceID    = true;
			else if (0 == line.find(KEYWORD_USE_INDIRECT_DRAW))    outDesc.bUseIndirectDraw       = true;
			else if (0 == line.find(KEYWORD_VPO_BEGIN))            outDesc.materialVPOBeginIx     = lineIx + 1;
			else if (0 == line.find(KEYWORD_VPO_END))              outDesc.materialVPOEndIx       = lineIx - 1;
			else if (0 == line.find(KEYWORD_ATTR_BEGIN))           outDesc.materialAttrBeginIx    = lineIx + 1;
			else if (0 == line.find(KEYWORD_ATTR_END))             outDesc.materialAttrEndIx      = lineIx - 1;
			else if (0 == line.find(KEYWORD_FORWARDSHADING_BEGIN)) outDesc.getSceneColorBeginIx   = lineIx + 1;
			else if (0 == line.find(KEYWORD_FORWARDSHADING_END))   outDesc.getSceneColorEndIx     = lineIx - 1;
			else if (0 == line.find(KEYWORD_EMBED_GLSL_BEGIN))     outDesc.embedGlslBeginIx       = lineIx + 1;
			else if (0 == line.find(KEYWORD_EMBED_GLSL_END))       outDesc.embedGlslEndIx         = lineIx - 1;
		}

		const bool bWellDefined =
			(outDesc.materialShadingModelIx != -1)
			&& (outDesc.materialVPOBeginIx != -1)
			&& (outDesc.materialVPOEndIx != -1)
			&& (outDesc.materialVPOBeginIx < outDesc.materialVPOEndIx)
			&& (outDesc.materialAttrBeginIx != -1)
			&& (outDesc.materialAttrEndIx != -1)
			&& (outDesc.materialAttrBeginIx < outDesc.materialAttrEndIx)
			&& (outDesc.getSceneColorBeginIx <= outDesc.getSceneColorEndIx)
			&& (outDesc.embedGlslBeginIx <= outDesc.embedGlslEndIx);
		CHECK(bWellDefined);
	}

	static EMaterialShadingModel parseShadingModel(const std::string& line) {
		EMaterialShadingModel shadingModel = EMaterialShadingModel::NUM_MODELS;

		char shadingModelStr[64];
		int ret = sscanf_s(line.c_str(), "#define SHADINGMODEL %s", shadingModelStr, (unsigned)_countof(shadingModelStr));
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

		return shadingModel;
	}

	static void assembleUniformBuffer(
		std::vector<ConstantParameterDesc>& inParameterDescs,
		std::string& outUniformBuffer,
		uint32& outUboTotalElements,
		std::vector<MaterialConstantParameter>& outParameters)
	{
		std::stringstream ubo("");
		uint32 uboTotalElements = 0; // 1 element = 4 bytes

		std::sort(inParameterDescs.begin(), inParameterDescs.end(),
			[](const ConstantParameterDesc& A, const ConstantParameterDesc& B) -> bool {
				return A.numElements > B.numElements;
			}
		);

		if (inParameterDescs.size() != 0) {
			ubo << "layout (std140, binding = UBO_BINDING_MATERIAL) uniform UBO_Material {\n";
			int32 uboPaddingId = 0;
			int32 numScalars = 0;
			uint32 uboCurrentOffset = 0;
			// #todo-material-assembler: More compact packing like (vec3 + float) or (vec2 + vec2).
			for (const ConstantParameterDesc& desc : inParameterDescs) {
				ubo << '\t';
				ubo << desc.toString();

				MaterialConstantParameter param;
				param.name = desc.name;
				param.datatype = desc.datatypeEnum;
				param.numElements = desc.numElements;
				param.uvalue[0] = param.uvalue[1] = param.uvalue[2] = param.uvalue[3] = 0;
				param.offset = uboCurrentOffset;
				outParameters.emplace_back(param);

				if (desc.numElements == 4) {
					ubo << '\n';
					uboTotalElements += 4;
					uboCurrentOffset += 16;
				} else if (desc.numElements == 3) {
					ubo << "\t\tfloat _unused";
					ubo << std::to_string(uboPaddingId++);
					ubo << ";\n";
					uboTotalElements += 4;
					uboCurrentOffset += 16;
				} else if (desc.numElements == 2) {
					ubo << "\t\tvec2 _unused";
					ubo << std::to_string(uboPaddingId++);
					ubo << ";\n";
					uboTotalElements += 4;
					uboCurrentOffset += 16;
				} else if (desc.numElements == 1) {
					ubo << '\n';
					uboTotalElements += 1;
					uboCurrentOffset += 4;
				} else {
					CHECK_NO_ENTRY();
				}
				numScalars += (desc.numElements == 1) ? 1 : 0;
			}
			switch (numScalars) {
			case 1:
				ubo << "\tvec3 _unused";
				ubo << std::to_string(uboPaddingId++);
				ubo << ";\n";
				uboTotalElements += 3;
				break;
			case 2:
				ubo << "\tvec2 _unused";
				ubo << std::to_string(uboPaddingId++);
				ubo << ";\n";
				uboTotalElements += 2;
				break;
			case 3:
				ubo << "\tfloat _unused";
				ubo << std::to_string(uboPaddingId++);
				ubo << ";\n";
				uboTotalElements += 1;
				break;
			default:
				break;
			}
			ubo << "} uboMaterial; // Total " + std::to_string(4 * uboTotalElements) + " bytes\n";
		}

		outUniformBuffer = ubo.str();
		outUboTotalElements = uboTotalElements;
	}

	static void assembleTextureParameters(
		std::vector<TextureParameterDesc>& inParameterDescs,
		std::string& outTextureParameters,
		std::vector<MaterialTextureParameter>& outParameters)
	{
		std::stringstream textures("");
		for (const TextureParameterDesc& desc : inParameterDescs) {
			textures << desc.toString();
			textures << '\n';

			MaterialTextureParameter param;
			param.name    = desc.name;
			param.binding = desc.binding;
			outParameters.emplace_back(param);
		}
		outTextureParameters = textures.str();
	}
}

// MaterialTemplate
namespace pathos {

	void MaterialTemplate::updatePlaceholderIx() {
		MaterialTemplate& MT = *this;
		const std::string NEED(NEED_HEADER);
		const int32 totalLines = (int32)MT.sourceLines.size();
		const std::pair<const char*, int* const> targetLines[] = {
			{ NEED_SHADERSTAGE          , &MT.lineIx_shaderstage          },
			{ NEED_SHADINGMODEL         , &MT.lineIx_shadingmodel         },
			{ NEED_OUTPUTWORLDNORMAL    , &MT.lineIx_outputworldnormal    },
			{ NEED_SKYBOXMATERIAL       , &MT.lineIx_skyboxmaterial       },
			{ NEED_TRANSFERDRAWID       , &MT.lineIx_transferdrawid       },
			{ NEED_TRANSFERINSTANCEID   , &MT.lineIx_transferinstanceid   },
			{ NEED_INDIRECT_DRAW_MODE   , &MT.lineIx_indirectdrawmode     },
			{ NEED_UBO                  , &MT.lineIx_ubo                  },
			{ NEED_TEXTUREPARAMETERS    , &MT.lineIx_textureParams        },
			{ NEED_VPO                  , &MT.lineIx_getVPO               },
			{ NEED_MATERIALATTRS        , &MT.lineIx_getMaterialAttrs     },
			{ NEED_GETSCENECOLOR        , &MT.lineIx_getSceneColor        },
			{ NEED_EMBEDGLSL            , &MT.lineIx_embedGlsl            },
		};
		for (int32 lineIx = 0; lineIx < totalLines; ++lineIx) {
			const std::string& line = MT.sourceLines[lineIx];
			if (line.find(NEED) == 0) {
				const std::string header = line.substr(NEED.size() + 1);
				for (int32 i = 0; i < _countof(targetLines); ++i) {
					if (header == targetLines[i].first) {
						*(targetLines[i].second) = lineIx;
						break;
					}
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

// MaterialShaderAssembler
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
		pathos::enumerateFiles(shaderDir.c_str(), false, files);
		for (const std::string& filename : files) {
			if (filename == MATERIAL_TEMPLATE_FILENAME) {
				continue;
			}
			
			std::string materialPath = shaderDir + filename;
			MaterialShader* material = new MaterialShader;
			CompileResponse response = generateMaterialProgram(material, materialPath.c_str(), filename.c_str(), false);
			CHECK(response != CompileResponse::Failed);
		}
	}

	void MaterialShaderAssembler::reloadMaterialShaders() {
		for (const auto& it : materialShaderMap) {
			MaterialShader* material = it.second;
			const char* fullpath = material->sourceFullpath.c_str();
			const char* filename = material->sourceFilename.c_str();
			CompileResponse response = generateMaterialProgram(material, fullpath, filename, true);
			CHECK(response != CompileResponse::Failed);
		}
	}

	MaterialShaderAssembler::CompileResponse MaterialShaderAssembler::generateMaterialProgram(MaterialShader* targetMaterial, const char* fullpath, const char* filename, bool isHotReload) {
		targetMaterial->sourceFullpath = fullpath;
		targetMaterial->sourceFilename = filename;

		std::fstream fileStream;
		fileStream.open(fullpath, std::ios::in);
		if (fileStream.is_open() == false) {
			LOG(LogError, "[Material] Failed to open: %s", fullpath);
			return CompileResponse::Failed;
		}
		
		std::string materialName = filename;
		materialName = materialName.substr(0, materialName.find_first_of('.'));

		const uint32 materialNameHash = COMPILE_TIME_CRC32_STR(materialName.c_str());
		if (!isHotReload) {
			CHECKF(materialShaderMap.find(materialNameHash) == materialShaderMap.end(), "Material name conflict");
		}

		// Gather original lines
		std::vector<std::string> materialLines;
		while (fileStream) {
			std::string line;
			std::getline(fileStream, line);
			materialLines.emplace_back(line);
		}

		auto assembleBlock = [&materialLines](int32 beginIx, int32 endIx) -> std::string {
			std::stringstream ss("");
			for (int32 ix = beginIx; ix <= endIx; ++ix) {
				ss << materialLines[ix];
				ss << '\n';
			}
			return ss.str();
		};

		// Parse parameter descriptions.
		const int32 totalMaterialLines = (int32)materialLines.size();
		std::vector<TextureParameterDesc> textureParams;
		std::vector<ConstantParameterDesc> constParams;
		parseMaterialParameters(materialLines, textureParams, constParams);

		std::string uniformBufferString;
		uint32 uboTotalElements = 0;
		std::vector<MaterialConstantParameter> materialConstParameters;
		assembleUniformBuffer(constParams, uniformBufferString, uboTotalElements, materialConstParameters);

		if (isHotReload && uboTotalElements * 4 != targetMaterial->uboTotalBytes) {
			LOG(LogError, "[Material] Reject hot reload due to different UBO sizes: %s", filename);
			return CompileResponse::RejectHotReload;
		}

		std::vector<MaterialTextureParameter> materialTextureParameters;
		std::string texturesString;
		assembleTextureParameters(textureParams, texturesString, materialTextureParameters);

		if (isHotReload) {
			bool changed = targetMaterial->textureParameters.size() != materialTextureParameters.size();
			if (!changed) {
				size_t n = targetMaterial->textureParameters.size();
				std::vector<uint32> A(n), B(n);
				for (size_t i = 0; i < n; ++i) {
					A[i] = targetMaterial->textureParameters[i].binding;
					B[i] = materialTextureParameters[i].binding;
				}
				std::sort(A.begin(), A.end());
				std::sort(B.begin(), B.end());
				changed = A != B;
			}
			if (changed) {
				LOG(LogError, "[Material] Reject hot reload due to different texture parameters: %s", filename);
				return CompileResponse::RejectHotReload;
			}
		}

		PlaceholderDesc placeholders;
		scanPlaceholders(materialLines, placeholders);
		bool bEmbmedGlslExists = placeholders.embedGlslBeginIx >= 0 && placeholders.embedGlslEndIx >= 0;
		EMaterialShadingModel shadingModel = parseShadingModel(materialLines[placeholders.materialShadingModelIx]);
		bool bForwardShading = (shadingModel == EMaterialShadingModel::TRANSLUCENT);
		bool bForwardShadingBlockExists = (placeholders.getSceneColorBeginIx != -1) && (placeholders.getSceneColorEndIx) != -1 && (placeholders.getSceneColorBeginIx < placeholders.getSceneColorEndIx);
		CHECK(!bForwardShading || bForwardShadingBlockExists);

		MaterialTemplate MT = materialTemplate->makeClone();

		MT.replaceShadingModel(materialLines[placeholders.materialShadingModelIx]);
		MT.replaceOutputWorldNormal(placeholders.bOutputWorldNormal ? "#define OUTPUTWORLDNORMAL 1" : "");
		MT.replaceSkyboxMaterial(placeholders.bSkyboxMaterial ? "#define SKYBOXMATERIAL 1" : "");
		MT.replaceTransferDrawID(placeholders.bTransferDrawID ? "#define TRANSFER_DRAW_ID 1" : "");
		MT.replaceTransferInstanceID(placeholders.bTransferInstanceID ? "#define TRANSFER_INSTANCE_ID 1" : "");
		MT.replaceIndirectDrawMode(placeholders.bUseIndirectDraw ? "#define INDIRECT_DRAW_MODE 1" : "");
		MT.replaceUBO(uniformBufferString);
		MT.replaceTextureParameters(texturesString);
		MT.replaceVPO(assembleBlock(placeholders.materialVPOBeginIx, placeholders.materialVPOEndIx));
		MT.replaceAttr(assembleBlock(placeholders.materialAttrBeginIx, placeholders.materialAttrEndIx));
		MT.replaceEmbedGlsl(bEmbmedGlslExists ? assembleBlock(placeholders.embedGlslBeginIx, placeholders.embedGlslEndIx) : "");
		MT.replaceGetSceneColor(bForwardShading ? assembleBlock(placeholders.getSceneColorBeginIx, placeholders.getSceneColorEndIx) : "");

		// Split newlines again (oneliner long UBOs cause strange shader compilation error).
		splitNewlines(MT.sourceLines);
		MT.updatePlaceholderIx();
		MT.fixupNewlines();

		targetMaterial->materialName          = std::move(materialName);
		targetMaterial->shadingModel          = shadingModel;
		targetMaterial->bTrivialDepthOnlyPass = placeholders.bTrivialDepthOnlyPass;
		targetMaterial->uboTotalBytes         = uboTotalElements * 4;
		targetMaterial->constantParameters    = std::move(materialConstParameters);
		targetMaterial->textureParameters     = std::move(materialTextureParameters);
		targetMaterial->generateShaderProgram(&MT, isHotReload);

		materialShaderMap[materialNameHash] = targetMaterial;

		return CompileResponse::Compiled;
	}

}
