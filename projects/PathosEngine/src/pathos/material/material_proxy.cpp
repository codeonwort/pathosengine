#include "material_proxy.h"
#include "material_shader.h"

namespace pathos {

	void MaterialProxy::fillUniformBuffer(uint8* uboMemory) {
		for (const MaterialConstantParameter& param : constantParameters) {
			switch (param.datatype) {
			case EMaterialParameterDataType::Float:
			{
				float* ptr = (float*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.fvalue[i];
			}
			break;
			case EMaterialParameterDataType::Int:
			{
				int32* ptr = (int32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.ivalue[i];
			}
			break;
			case EMaterialParameterDataType::Uint:
			{
				uint32* ptr = (uint32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.uvalue[i];
			}
			break;
			case EMaterialParameterDataType::Bool:
			{
				uint32* ptr = (uint32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = (uint32)param.bvalue[i];
			}
			break;
			default:
				CHECK_NO_ENTRY();
				break;
			}
		}
	}

	EMaterialShadingModel MaterialProxy::getShadingModel() const {
		return materialShader->shadingModel;
	}

}
