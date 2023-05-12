#pragma once

#include "badger/types/int_types.h"
#include <vector>
#include <string>
#include <map>

namespace pathos {

	class MaterialShader;

	struct MaterialTemplate {

		bool checkPlaceholders() const {
			return lineIx_shaderstage != -1
				&& lineIx_shadingmodel != -1
				&& lineIx_outputworldnormal != -1 // Optional
				&& lineIx_skyboxmaterial != -1 // Optional
				&& lineIx_ubo != -1
				&& lineIx_textureParams != -1
				&& lineIx_getVPO != -1
				//&& lineIx_getSceneColor != -1 // Forward shading only
				&& lineIx_getMaterialAttrs != -1;
		}

		void updatePlaceholderIx();

		void fixupNewlines() {
			for (std::string& line : sourceLines) {
				if (line.size() == 0 || line[line.size() - 1] != '\n') {
					line += '\n';
				}
			}
		}

		MaterialTemplate makeClone() {
			return *this;
		}

		void replaceShaderStage(const std::string& defineShaderStage) {
			sourceLines[lineIx_shaderstage] = defineShaderStage;
		}
		void replaceShadingModel(const std::string& defineShadingModel) {
			sourceLines[lineIx_shadingmodel] = defineShadingModel;
		}
		void replaceOutputWorldNormal(const std::string& outputWorldNormal) {
			sourceLines[lineIx_outputworldnormal] = outputWorldNormal;
		}
		void replaceSkyboxMaterial(const std::string& skyboxMaterial) {
			sourceLines[lineIx_skyboxmaterial] = skyboxMaterial;
		}
		void replaceUBO(const std::string& defineUBO) {
			sourceLines[lineIx_ubo] = defineUBO;
		}
		void replaceTextureParameters(const std::string& parameters) {
			sourceLines[lineIx_textureParams] = parameters;
		}
		void replaceVPO(const std::string& vpo) {
			sourceLines[lineIx_getVPO] = vpo;
		}
		void replaceAttr(const std::string& attr) {
			sourceLines[lineIx_getMaterialAttrs] = attr;
		}
		void replaceGetSceneColor(const std::string& getSceneColor) {
			// Only valid for forward shading.
			CHECK(lineIx_getSceneColor != -1);
			sourceLines[lineIx_getSceneColor] = getSceneColor;
		}

		int32 lineIx_shaderstage = -1;
		int32 lineIx_shadingmodel = -1;
		int32 lineIx_outputworldnormal = -1;
		int32 lineIx_skyboxmaterial = -1;
		int32 lineIx_ubo = -1;
		int32 lineIx_textureParams = -1;
		int32 lineIx_getVPO = -1;
		int32 lineIx_getMaterialAttrs = -1;
		int32 lineIx_getSceneColor = -1;
		std::vector<std::string> sourceLines;
	};

	// Generates material shaders based on material templates.
	class MaterialShaderAssembler final {

	public:
		static MaterialShaderAssembler& get();

		void initializeMaterialShaders();

		MaterialShader* findMaterialShaderByName(const char* materialName);
		MaterialShader* findMaterialShaderByHash(uint32 materialNameHash);

	private:
		// Load material template files.
		void loadMaterialTemplate();

		// Enumerate material files and call generateMaterialShader().
		void parseAllMaterialShaders();

		// Parse material shader file to generate corresponding shader program.
		MaterialShader* generateMaterialShader(const char* fullpath, const char* filename);

	private:
		MaterialShaderAssembler() = default;
		~MaterialShaderAssembler();

		MaterialTemplate* materialTemplate = nullptr;
		bool bTemplateLoaded = false;

		std::map<uint32, MaterialShader*> materialShaderMap;

	};

}
