#pragma once

#include "material_id.h"
#include "material_parameter.h"

#include <vector>

namespace pathos {

	class MaterialShader;

	class MaterialProxy {

	public:
		struct UBO_PerObject {
			static constexpr uint32 BINDING_POINT = 1;

			// #todo-material: Upload only object ID. Read model transform from some buffer.
			matrix4 modelTransform;
			matrix4 prevModelTransform;
		};

		void fillUniformBuffer(uint8* uboMemory);

		EMaterialShadingModel getShadingModel() const;

	public:
		MaterialShader* materialShader;
		uint32          materialInstanceID;
		bool            bWireframe;

	public:
		std::vector<MaterialConstantParameter> constantParameters;
		std::vector<MaterialTextureParameter> textureParameters;
	};

}
