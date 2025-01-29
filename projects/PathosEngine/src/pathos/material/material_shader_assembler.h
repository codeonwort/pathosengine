#pragma once

#include "badger/types/int_types.h"
#include <vector>
#include <string>
#include <map>

namespace pathos {

	class MaterialShader;

	struct MaterialTemplate {

		// Check if _template.glsl is valid.
		bool checkPlaceholders() const {
			return lineIx_shaderstage          != -1
				&& lineIx_shadingmodel         != -1
				&& lineIx_outputworldnormal    != -1 // Optional
				&& lineIx_skyboxmaterial       != -1 // Optional
				&& lineIx_transferdrawid       != -1 // Optional
				&& lineIx_transferinstanceid   != -1 // Optional
				&& lineIx_indirectdrawmode     != -1 // Optional
				&& lineIx_ubo                  != -1
				&& lineIx_textureParams        != -1
				&& lineIx_getVPO               != -1
				//&& lineIx_getSceneColor != -1 // Forward shading only
				&& lineIx_embedGlsl            != -1 // Optional
				&& lineIx_getMaterialAttrs     != -1;
		}

		// Find line numbers that start with a $NEED token.
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
		void replaceTransferDrawID(const std::string& defineTransferDrawID) {
			sourceLines[lineIx_transferdrawid] = defineTransferDrawID;
		}
		void replaceTransferInstanceID(const std::string& defineTransferInstanceID) {
			sourceLines[lineIx_transferinstanceid] = defineTransferInstanceID;
		}
		void replaceIndirectDrawMode(const std::string& indirectDrawMode) {
			sourceLines[lineIx_indirectdrawmode] = indirectDrawMode;
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
		void replaceEmbedGlsl(const std::string& embedGlsl) {
			sourceLines[lineIx_embedGlsl] = embedGlsl;
		}

		int32 lineIx_shaderstage           = -1;
		int32 lineIx_shadingmodel          = -1;
		int32 lineIx_outputworldnormal     = -1;
		int32 lineIx_skyboxmaterial        = -1;
		int32 lineIx_transferdrawid        = -1;
		int32 lineIx_transferinstanceid    = -1;
		int32 lineIx_indirectdrawmode      = -1;
		int32 lineIx_ubo                   = -1;
		int32 lineIx_textureParams         = -1;
		int32 lineIx_getVPO                = -1;
		int32 lineIx_getMaterialAttrs      = -1;
		int32 lineIx_getSceneColor         = -1;
		int32 lineIx_embedGlsl             = -1;
		std::vector<std::string> sourceLines; // Source code of _template.glsl
	};

	// Generates material shaders based on material templates.
	class MaterialShaderAssembler final {
		enum class CompileResponse : uint8 { Failed, RejectHotReload, Compiled };

	public:
		static MaterialShaderAssembler& get();

		void initializeMaterialShaders();

		// Material hot reloading is handled here, instead of in ShaderProgram::reload().
		void reloadMaterialShaders();

		MaterialShader* findMaterialShaderByName(const char* materialName);
		MaterialShader* findMaterialShaderByHash(uint32 materialNameHash);

	private:
		// Load material template files.
		void loadMaterialTemplate();

		// Enumerate material files and call generateMaterialShader().
		void parseAllMaterialShaders();

		// Generate material program from given file.
		CompileResponse generateMaterialProgram(MaterialShader* targetMaterial, const char* fullpath, const char* filename, bool isHotReload);

	private:
		MaterialShaderAssembler() = default;
		~MaterialShaderAssembler();

		MaterialTemplate* materialTemplate = nullptr;
		bool bTemplateLoaded = false;

		std::map<uint32, MaterialShader*> materialShaderMap;

	};

}
