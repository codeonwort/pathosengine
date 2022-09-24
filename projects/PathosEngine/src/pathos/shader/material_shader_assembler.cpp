#include "material_shader_assembler.h"
#include "pathos/util/file_system.h"
#include "pathos/util/resource_finder.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#define MATERIAL_FOLDER            "shaders/materials/"
#define MATERIAL_TEMPLATE_FILENAME "_template.glsl"

// Things that are filled by material shaders
#define NEED_HEADER                "$NEED"
#define NEED_SHADINGMODEL          "SHADINGMODEL"
#define NEED_VPO                   "getVertexPositionOffset"

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

}
