#include "pch.h"
#include "CppUnitTest.h"

#include "pathos/engine.h"
#include "pathos/scene/camera.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pathos;

namespace UnitTest
{
	TEST_CLASS(TestCamera)
	{
	public:
		
		TEST_METHOD(TestEyeVector)
		{
			PerspectiveLens lens(60.0f, 1.0f, 0.1f, 5000.0f);
			Camera camera(lens);
			camera.lookAt(vector3(0.0f, 0.0f, 10.0f), vector3(0.0f, 0.0f, 0.0f), vector3(0.f, 1.0f, 0.0f));

			bool equals = camera.getEyeVector() == vector3(0.0f, 0.0f, -1.0f);

			Assert::IsTrue(equals, L"Camera eye vector is wrong");
		}
	};
}
