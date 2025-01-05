#include "pch.h"
#include "CppUnitTest.h"

#include "badger/math/signed_volume.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(TestSignedVolume)
	{
	public:
		TEST_METHOD(TestSignedVolume1D)
		{
			const vector3 s1(-1.0f, 1.0f, 0.0f);
			const vector3 s2(1.0f, 1.0f, 0.0f);
			const vector3 p(0.0f, 0.0f, 0.0f);

			vector3 p0 = badger::projectPointOntoLine(s1, s2, p);
			bool bProjected = p0 == vector3(0.0f, 1.0f, 0.0f);
			Assert::IsTrue(bProjected, L"Projection of point onto line is invalid.");
			
			vector2 uv = badger::signedVolume1D(s1, s2);
			p0 = uv.x * s1 + uv.y * s2;
			bProjected = p0 == vector3(0.0f, 1.0f, 0.0f);
			Assert::IsTrue(bProjected, L"Signed volume 1D is invalid.");
		}
	};
}
