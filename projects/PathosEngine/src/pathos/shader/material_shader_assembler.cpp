#include "material_shader_assembler.h"
#include "material_shader.h"
#include "pathos/util/file_system.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include <fstream>
#include <string>
#include <vector>

#define MATERIAL_FOLDER            "shaders/materials/"
#define MATERIAL_TEMPLATE_FILENAME "_template.glsl"

// Things that are filled by material shaders
#define NEED_HEADER                "$NEED"
#define NEED_SHADINGMODEL          "SHADINGMODEL"
#define NEED_VPO                   "getVertexPositionOffset"

// Macros that are used by material shaders
#define PARAMETER_CONSTANT         "PARAMETER_CONSTANT"
#define PARAMETER_TEXTURE          "PARAMETER_TEXTURE"

namespace pathos {

	struct MaterialTemplate {

		bool checkPlaceholders() const {
			return lineIx_shadingmodel != -1
				&& lineIx_getVPO != -1;
		}

		int32 lineIx_shadingmodel = -1;
		int32 lineIx_getVPO = -1;
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
				} else if (header == NEED_VPO) {
					MT.lineIx_getVPO = lineIx;
				}
			}
		}
		CHECK(MT.checkPlaceholders());

		bTemplateLoaded = true;
	}

	void MaterialShaderAssembler::parseAllMaterialShaders() {
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
				// Texture parameter: (binding, samplerType, textureName)
			} else if (line.find(PARAMETER_CONSTANT) == 0) {
				char datatype[16];
				char name[64];
				int ret = sscanf_s(line.c_str(),
					"PARAMETER_CONSTANT(%[^,], %[^)])",
					datatype, (unsigned)_countof(datatype),
					name, (unsigned)_countof(name));
				CHECK(ret == 2);
				// Constant parameter: (datatype, name)
			}
		}

		// #todo-material-assembler
		// 1. make a clone of the template.
		// 2. Replace placeholders with current material shader.

		// #todo-material-assembler: Dump material shaders for debugging
		{
			std::string dumppath = pathos::getSolutionDir();
			dumppath += "log/material_dump/";
			pathos::createDirectory(dumppath.c_str());
			dumppath += filename;

			std::fstream fs;
			fs.open(dumppath, std::ios::out);
			for (const std::string& line : sourceLines) {
				fs << line << '\n';
			}
			fs.close();
		}

		return nullptr;
	}

}
