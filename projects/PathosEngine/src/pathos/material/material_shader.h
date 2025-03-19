#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/material/material_id.h"
#include "pathos/material/material_parameter.h"

#include "badger/types/vector_types.h"
#include <vector>
#include <string>

namespace pathos {

	struct MaterialTemplate;
	class ShaderProgram;
	class Texture;

	class MaterialShader {
		friend class MaterialShaderAssembler;

	public:
		void generateShaderProgram(const MaterialTemplate* materialTemplate, bool isHotReload);

		// Copy material parameters into Material.
		void extractMaterialParameters(
			std::vector<MaterialConstantParameter>& outConstants,
			std::vector<MaterialTextureParameter>& outTextures);

		uint32 getNextInstanceID();

	public:
		std::string materialName;
		EMaterialShadingModel shadingModel;
		bool bTrivialDepthOnlyPass = true;
		uint32 uboTotalBytes = 0;

		ShaderProgram* program = nullptr;
		uint32 programHash = 0;

		UniformBuffer uboMaterial;
		std::string uboName;
		// #todo-material: See _template.glsl
		const uint32 uboBindingPoint = 2;

		std::string nameVS;
		std::string nameFS;

		std::string sourceFullpath;
		std::string sourceFilename;
		std::vector<std::string> sourceBackupVS;
		std::vector<std::string> sourceBackupFS;

	private:
		uint32 lastInstanceID = 0;

		// These are just kept as original data.
		// Actual parameter values are controlled by Material.
		std::vector<MaterialConstantParameter> constantParameters;
		std::vector<MaterialTextureParameter> textureParameters;
	};

}
