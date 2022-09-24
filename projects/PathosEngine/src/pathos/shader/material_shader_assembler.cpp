#include "material_shader_assembler.h"
#include "material_shader.h"
#include "pathos/util/file_system.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#define MATERIAL_FOLDER            "shaders/materials/"
#define MATERIAL_TEMPLATE_FILENAME "_template.glsl"

// Things that are filled by material shaders
#define NEED_HEADER                "$NEED"
#define NEED_SHADINGMODEL          "SHADINGMODEL"
#define NEED_UBO                   "UBO_Material"
#define NEED_TEXTUREPARAMETERS     "TEXTURE_PARAMETERS"
#define NEED_VPO                   "getVertexPositionOffset"
#define NEED_MATERIALATTRS         "getMaterialAttributes"

// Macros that are used by material shaders
#define PARAMETER_CONSTANT         "PARAMETER_CONSTANT"
#define PARAMETER_TEXTURE          "PARAMETER_TEXTURE"

namespace pathos {

	struct MaterialTemplate {

		bool checkPlaceholders() const {
			return lineIx_shadingmodel != -1
				&& lineIx_ubo != -1
				//&& lineIx_textureParams != -1 // Not mandatory
				&& lineIx_getVPO != -1
				&& lineIx_getMaterialAttrs != -1;
		}

		MaterialTemplate makeClone() {
			// #todo-material-assembler: Change if any member does not deep copy.
			return *this;
		}

		void replaceShadingModel(const std::string& defineShadingModel) {
			sourceLines[lineIx_shadingmodel] = defineShadingModel;
		}
		void replaceUBO(const std::string& defineUBO) {
			sourceLines[lineIx_ubo] = defineUBO;
		}
		void replaceTextureParameters(const std::string& parameters) {
			sourceLines[lineIx_textureParams] = parameters;
		}

		int32 lineIx_shadingmodel = -1;
		int32 lineIx_ubo = -1;
		int32 lineIx_textureParams = -1;
		int32 lineIx_getVPO = -1;
		int32 lineIx_getMaterialAttrs = -1;
		std::vector<std::string> sourceLines;
	};

}

namespace pathos {

	MaterialShaderAssembler& MaterialShaderAssembler::get() {
		static MaterialShaderAssembler instance;
		return instance;
	}

	MaterialShaderAssembler::~MaterialShaderAssembler() {
		if (materialTemplate != nullptr) {
			delete materialTemplate;
		}
	}

	void MaterialShaderAssembler::loadMaterialTemplate() {
		if (bTemplateLoaded) {
			return;
		}

		std::string templatePathRel = MATERIAL_FOLDER;
		templatePathRel += MATERIAL_TEMPLATE_FILENAME;
		std::string templatePath = ResourceFinder::get().find(templatePathRel);
		templatePath = pathos::getAbsolutePath(templatePath.c_str());
		if (templatePath.size() == 0) {
			char msg[256];
			sprintf_s(msg, "Can't find material template: %s", templatePathRel.c_str());
			CHECKF(false, msg);
		}

		std::fstream fileStream;
		fileStream.open(templatePath.c_str(), std::ios::in);
		CHECKF(fileStream.is_open(), "Failed to open a filestream");

		materialTemplate = new MaterialTemplate;
		MaterialTemplate& MT = *materialTemplate;

		// Gather original lines
		while (fileStream) {
			std::string line;
			std::getline(fileStream, line);
			MT.sourceLines.emplace_back(line);
		}

		// Find $NEED lines
		const std::string NEED(NEED_HEADER);
		const int32 totalLines = (int32)MT.sourceLines.size();
		for (int32 lineIx = 0; lineIx < totalLines; ++lineIx) {
			const std::string& line = MT.sourceLines[lineIx];
			if (line.find(NEED) == 0) {
				std::string header = line.substr(NEED.size() + 1);
				if (header == NEED_SHADINGMODEL) {
					MT.lineIx_shadingmodel = lineIx;
				} else if (header == NEED_UBO) {
					MT.lineIx_ubo = lineIx;
				} else if (header == NEED_TEXTUREPARAMETERS) {
					MT.lineIx_textureParams = lineIx;
				} else if (header == NEED_VPO) {
					MT.lineIx_getVPO = lineIx;
				} else if (header == NEED_MATERIALATTRS) {
					MT.lineIx_getMaterialAttrs = lineIx;
				}
			}
		}
		CHECK(MT.checkPlaceholders());

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
			// #todo-material-assembler: Replace with fallback material
			CHECK(material != nullptr);
			materials.push_back(material);
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
				sprintf_s(buf, "layout (binding = %u) uniform %s %s",
					binding, samplerType.c_str(), name.c_str());
				return std::string(buf);
			}
		};
		struct ConstantParameterDesc {
			uint32 lineIx;
			std::string datatype;
			std::string name;
			uint32 numBytes;

			ConstantParameterDesc(uint32 inLineIx, const std::string& inDatatype, const std::string& inName)
				: lineIx(inLineIx), datatype(inDatatype), name(inName)
			{
				if (datatype == "float" || datatype == "int" || datatype == "uint" || datatype == "bool") {
					numBytes = 1;
				} else if (datatype == "vec2" || datatype == "ivec2" || datatype == "uvec2" || datatype == "bvec2") {
					numBytes = 2;
				} else if (datatype == "vec3" || datatype == "ivec3" || datatype == "uvec3" || datatype == "bvec3") {
					numBytes = 3;
				} else if (datatype == "vec4" || datatype == "ivec4" || datatype == "uvec4" || datatype == "bvec4") {
					numBytes = 4;
				} else {
					numBytes = 0;
					CHECK_NO_ENTRY();
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

		// Gather original lines
		std::vector<std::string> sourceLines;
		while (fileStream) {
			std::string line;
			std::getline(fileStream, line);
			sourceLines.emplace_back(line);
		}

		// Parse parameters
		const int32 totalLines = (int32)sourceLines.size();
		std::vector<TextureParameterDesc> textureParams;
		std::vector<ConstantParameterDesc> constParams;
		for (int32 lineIx = 0; lineIx < totalLines; ++lineIx) {
			const std::string& line = sourceLines[lineIx];
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
				int ret = sscanf_s(line.c_str(),
					"PARAMETER_CONSTANT(%[^,], %[^)])",
					datatype, (unsigned)_countof(datatype),
					name, (unsigned)_countof(name));
				CHECK(ret == 2);

				constParams.push_back(
					ConstantParameterDesc((uint32)lineIx, datatype, name));
			}
		}

		// Find definitions
		int32 materialShadingModelIx = -1;
		for (int32 lineIx = 0; lineIx < totalLines; ++lineIx) {
			const std::string& line = sourceLines[lineIx];
			if (0 == line.find("#define SHADINGMODEL")) {
				materialShadingModelIx = lineIx;
			}
		}
		CHECK(materialShadingModelIx != -1);

		// #todo-material-assembler
		// 1. Make a clone of the template.
		// 2. Replace placeholders with current material shader.
		MaterialTemplate MT = materialTemplate->makeClone();
		MT.replaceShadingModel(sourceLines[materialShadingModelIx]);

		// Construct material uniform buffer.
		std::sort(constParams.begin(), constParams.end(),
			[](ConstantParameterDesc& A, ConstantParameterDesc& B) -> bool {
				return A.numBytes > B.numBytes;
			}
		);
		std::string ubo = "layout (std140, binding = UBO_MATERIAL_BINDING_POINT) uniform UBO_Material {\n";
		int32 uboPaddingId = 0;
		int32 numScalars = 0;
		uint32 uboTotalElements = 0; // 1 element = 4 bytes
		// #todo-material-assembler: More compact packing like (vec3 + float) or (vec2 + vec2).
		for (const ConstantParameterDesc& desc : constParams) {
			ubo += '\t';
			ubo += desc.toString();
			if (desc.numBytes == 4) {
				ubo += '\n';
				uboTotalElements += 4;
			} else if (desc.numBytes == 3) {
				ubo += "\t\tfloat _unused";
				ubo += std::to_string(uboPaddingId++);
				ubo += ";\n";
				uboTotalElements += 4;
			} else if (desc.numBytes == 2) {
				ubo += "\t\tvec2 _unused";
				ubo += std::to_string(uboPaddingId++);
				ubo += ";\n";
				uboTotalElements += 4;
			} else if (desc.numBytes == 1) {
				ubo += '\n';
				uboTotalElements += 1;
			} else {
				CHECK_NO_ENTRY();
			}
			numScalars += (desc.numBytes == 1) ? 1 : 0;
		}
		switch(numScalars){
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
		ubo += "} ubo; // Total " + std::to_string(4 * uboTotalElements) + " bytes\n";
		MT.replaceUBO(ubo);

		if (textureParams.size() == 0) {
			MT.replaceTextureParameters("");
		} else {
			std::string textures;
			for (const TextureParameterDesc& desc : textureParams) {
				textures += desc.toString();
				textures += '\n';
			}
			MT.replaceTextureParameters(textures);
		}

		// #todo-material-assembler
		// Replace getVertexPositionOffset
		// Replace getMaterialAttributes

		std::string materialName = filename;
		materialName = materialName.substr(0, materialName.find_first_of('.'));

		// #todo-material-assembler: Dump material shaders for debugging
		{
			std::string dumppath = pathos::getSolutionDir();
			dumppath += "log/material_dump/";
			pathos::createDirectory(dumppath.c_str());
			dumppath += materialName + ".dump.glsl";

			std::fstream fs;
			fs.open(dumppath, std::ios::out);
			for (const std::string& line : MT.sourceLines) {
				fs << line << '\n';
			}
			fs.close();
		}

		MaterialShader* shader = new MaterialShader(materialName);
		//

		return shader;
	}

}
