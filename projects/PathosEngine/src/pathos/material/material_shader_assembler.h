#pragma once

#include "material_id.h"
#include "material_parameter.h"

#include "badger/types/int_types.h"
#include <vector>
#include <string>
#include <map>

// Intermediate data for MaterialShaderAssembler
namespace pathos {
	namespace material {

		struct TextureParameterDesc {
			uint32 lineIx;
			uint32 binding;
			std::string samplerType;
			std::string name;

			std::string toString() const;
		};

		struct ConstantParameterDesc {
			uint32 lineIx;
			std::string datatype;
			std::string name;
			uint32 numElements;
			EMaterialParameterDataType datatypeEnum = EMaterialParameterDataType::Float;

			ConstantParameterDesc(uint32 inLineIx, const std::string& inDatatype, const std::string& inName);

			std::string toString() const;
		};

		void parseMaterialParameters(
			const std::vector<std::string>& materialLines,
			std::vector<TextureParameterDesc>& outTextureParams,
			std::vector<ConstantParameterDesc>& outConstParams);
	}
}

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

		MaterialTemplate makeClone() const {
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
		enum class ParserStatus : uint8 { FileNotFound, CannotHotReloadUBO, CannotHotReloadTextures, Success };
		struct HotReloadContext {
			bool bHotReload = false;
			uint32 uboTotalBytes = 0;
			std::vector<MaterialTextureParameter>* textureParameters = nullptr; // Should not be null if bHotReload == true
		};
		struct ParserOutput {
			ParserStatus status;
			// Source lines
			MaterialTemplate MT;
			// Characteristics
			EMaterialShadingModel shadingModel;
			bool bForwardShading;
			bool bForwardShadingBlockExists;
			bool bTrivialDepthOnlyPass;
			// Shader parameters
			uint32 uboTotalElements;
			std::vector<MaterialConstantParameter> materialConstParameters;
			std::vector<MaterialTextureParameter> materialTextureParameters;
		};

	public:
		static MaterialShaderAssembler& get();
		static MaterialTemplate* loadMaterialTemplateFromFile();
		// @param outResult     The result. Check outResult->status to see if it succeeded.
		// @param prototypeMT   A material template returned by loadMaterialTemplateFromFile()
		// @param hotReloadCtx  If hot-reloading, provide this context
		// @param fullpath      Shader full filepath (e.g., "C:/Workspace/Pathos/shaders/materials/gltf_opaque.glsl")
		static void parseMaterialProgram(ParserOutput* outResult, const MaterialTemplate* prototypeMT, const HotReloadContext& hotReloadCtx, const char* fullpath);

		void initializeMaterialShaders();

		// Material hot reloading is handled here, instead of in ShaderProgram::reload().
		void reloadMaterialShaders();

		MaterialShader* findMaterialShaderByName(const char* materialName);
		MaterialShader* findMaterialShaderByHash(uint32 materialNameHash);

	private:
		// Load material template file.
		void loadMaterialTemplate();

		// Enumerate material files and call generateMaterialShader().
		void parseAllMaterialShaders();

		// Generate material program from given file.
		CompileResponse generateMaterialProgram(MaterialShader* targetMaterial, const char* fullpath, const char* filename, bool isHotReload);

	private:
		MaterialShaderAssembler() = default;
		~MaterialShaderAssembler();

		MaterialTemplate* prototypeMT = nullptr;
		bool bTemplateLoaded = false;

		std::map<uint32, MaterialShader*> materialShaderMap;

	};

}
