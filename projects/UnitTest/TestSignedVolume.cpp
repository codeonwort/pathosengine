#include "pch.h"
#include "CppUnitTest.h"

#include "badger/math/signed_volume.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(TestSignedVolume) {
	public:
		TEST_METHOD(TestSignedVolume1D) {
			// In between
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

			// Far side of s1
			{
				const vector3 s1(1.0f, 1.0f, 0.0f);
				const vector3 s2(3.0f, 2.0f, 0.0f);
				const vector3 p(0.0f, 0.0f, 0.0f);

				vector2 uv = badger::signedVolume1D(s1, s2);
				vector3 p0 = uv.x * s1 + uv.y * s2;
				Assert::IsTrue(uv == vector2(1.0f, 0.0f), L"Barycentric coordinates are invalid.");
				Assert::IsTrue(p0 == s1, L"Signed volume 1D is invalid.");
			}

			// Far side of s2
			{
				const vector3 s1(-5.0f, 1.0f, -2.0f);
				const vector3 s2(-2.0f, 1.0f, -2.0f);
				const vector3 p(0.0f, 0.0f, 0.0f);

				vector2 uv = badger::signedVolume1D(s1, s2);
				vector3 p0 = uv.x * s1 + uv.y * s2;
				Assert::IsTrue(uv == vector2(0.0f, 1.0f), L"Barycentric coordinates are invalid.");
				Assert::IsTrue(p0 == s2, L"Signed volume 1D is invalid.");
			}
		}

		TEST_METHOD(TestSignedVolume3D) {
			const vector3 originalPoints[4] = {
				vector3(0.0f, 0.0f, 0.0f),
				vector3(1.0f, 0.0f, 0.0f),
				vector3(0.0f, 1.0f, 0.0f),
				vector3(0.0f, 0.0f, 1.0f)
			};
			vector3 points[4];
			vector4 lambdas;
			vector3 v;
			wchar_t msg[1024];

			auto updateValues = [&]() {
				lambdas = badger::signedVolume3D(points[0], points[1], points[2], points[3]);
				v = vector3(0.0f);
				for (int32 i = 0; i < 4; ++i) v += points[i] * lambdas[i];
			};
			
			auto vec4Equals = [](const vector4& a, const vector4& b) {
				constexpr float eps = (float)1e-6; //std::numeric_limits<float>::epsilon(); // too tight?
				return glm::distance(a, b) <= eps;
			};
			auto vec3Equals = [](const vector3& a, const vector3& b) {
				constexpr float eps = (float)1e-6; //std::numeric_limits<float>::epsilon(); // too tight?
				return glm::distance(a, b) <= eps;
			};

			// Case 1
			for (int32 i = 0; i < 4; ++i) {
				points[i] = originalPoints[i] + vector3(1.0f, 1.0f, 1.0f);
			}
			updateValues();
			Assert::IsTrue(lambdas == vector4(1.0f, 0.0f, 0.0f, 0.0f), L"SignedVolume3D is invalid");
			Assert::IsTrue(v == vector3(1.0f, 1.0f, 1.0f), L"SignedVolume3D is invalid");

			// Case 2
			for (int32 i = 0; i < 4; ++i) {
				points[i] = originalPoints[i] + vector3(-0.25f, -0.25f, -0.25f);
			}
			updateValues();
			swprintf_s(msg, L"lambdas = (%f, %f, %f, %f)", lambdas.x, lambdas.y, lambdas.z, lambdas.w);
			Assert::IsTrue(vec4Equals(lambdas, vector4(0.25f)), msg);
			swprintf_s(msg, L"v = (%f, %f, %f)", v.x, v.y, v.z);
			Assert::IsTrue(vec3Equals(v, vector3(0.0f)), msg);

			// Case 3
			for (int32 i = 0; i < 4; ++i) {
				points[i] = originalPoints[i] + vector3(-1.0f, -1.0f, -1.0f);
			}
			updateValues();
			swprintf_s(msg, L"lambdas = (%f, %f, %f, %f)", lambdas.x, lambdas.y, lambdas.z, lambdas.w);
			Assert::IsTrue(vec4Equals(lambdas, vector4(0.0f, 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f)), msg);
			swprintf_s(msg, L"v = (%f, %f, %f)", v.x, v.y, v.z);
			Assert::IsTrue(vec3Equals(v, vector3(-2.0f / 3.0f)), msg);

			// Case 4
			for (int32 i = 0; i < 4; ++i) {
				points[i] = originalPoints[i] + vector3(1.0f, 1.0f, -0.5f);
			}
			updateValues();
			swprintf_s(msg, L"lambdas = (%f, %f, %f, %f)", lambdas.x, lambdas.y, lambdas.z, lambdas.w);
			Assert::IsTrue(vec4Equals(lambdas, vector4(0.5f, 0.0f, 0.0f, 0.5f)), msg);
			swprintf_s(msg, L"v = (%f, %f, %f)", v.x, v.y, v.z);
			Assert::IsTrue(vec3Equals(v, vector3(1.0f, 1.0f, 0.0f)), msg);

			// Case 5
			points[0] = vector3(51.1996613f, 26.1989613f, 1.91339576f);
			points[1] = vector3(-51.0567360f, -26.0565681f, -0.436143428f);
			points[2] = vector3(50.8978920f, -24.1035538f, -1.04042661);
			points[3] = vector3(-49.1021080f, 25.8964462f, -1.04042661f);
			updateValues();
			swprintf_s(msg, L"lambdas = (%f, %f, %f, %f)", lambdas.x, lambdas.y, lambdas.z, lambdas.w);
			Assert::IsTrue(vec4Equals(lambdas, vector4(0.290401f, 0.302230f, 0.205651f, 0.201718f)), msg);
			swprintf_s(msg, L"v = (%f, %f, %f)", v.x, v.y, v.z);
			Assert::IsTrue(vec3Equals(v, vector3(0.0f, 0.0f, 0.0f)), msg);
		}
	};
}
