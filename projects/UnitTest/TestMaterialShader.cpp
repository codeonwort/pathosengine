#include "pch.h"
#include "CppUnitTest.h"

#include "pathos/material/material_shader_assembler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pathos;

namespace UnitTest
{
	TEST_CLASS(TestMaterialShader)
	{
	public:
		TEST_METHOD(ParseMaterialParameter)
		{
			std::vector<std::string> source = {
				"PARAMETER_TEXTURE(0, sampler2D, albedoTexture)",
				"PARAMETER_TEXTURE(1, sampler2D, normalTexture)",
				"PARAMETER_CONSTANT(vec3, albedo)",
				"PARAMETER_CONSTANT(float, metallic)",
				"PARAMETER_CONSTANT(uvec2, resolution)",
			};
			std::vector<pathos::material::TextureParameterDesc> textureParams;
			std::vector<pathos::material::ConstantParameterDesc> constParams;

			pathos::material::parseMaterialParameters(source, textureParams, constParams);

			Assert::IsTrue(textureParams[0].binding == 0);
			Assert::IsTrue(textureParams[0].samplerType == "sampler2D");
			Assert::IsTrue(textureParams[0].name == "albedoTexture");

			Assert::IsTrue(textureParams[1].binding == 1);
			Assert::IsTrue(textureParams[1].samplerType == "sampler2D");
			Assert::IsTrue(textureParams[1].name == "normalTexture");

			Assert::IsTrue(constParams[0].datatype == "vec3");
			Assert::IsTrue(constParams[0].name == "albedo");
			Assert::IsTrue(constParams[0].datatypeEnum == EMaterialParameterDataType::Float);
			Assert::IsTrue(constParams[0].numElements == 3);

			Assert::IsTrue(constParams[1].datatype == "float");
			Assert::IsTrue(constParams[1].name == "metallic");
			Assert::IsTrue(constParams[1].datatypeEnum == EMaterialParameterDataType::Float);
			Assert::IsTrue(constParams[1].numElements == 1);

			Assert::IsTrue(constParams[2].datatype == "uvec2");
			Assert::IsTrue(constParams[2].name == "resolution");
			Assert::IsTrue(constParams[2].datatypeEnum == EMaterialParameterDataType::Uint);
			Assert::IsTrue(constParams[2].numElements == 2);
		}
	};
}
