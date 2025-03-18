#include "pch.h"
#include "CppUnitTest.h"

#include <math.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pathos;

namespace {
	#define PI         3.14159265359
	#define TWO_PI     6.28318530718
	using vec2 = vector2;
	using vec3 = vector3;
	float atan(float y, float x) { return std::atan2f(y, x); }

	// Maps a 3D unit direction vector to a 2D uv coordinate.
	// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
	vec2 CubeToEquirectangular(vec3 v) {
		vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
		uv *= vec2(0.1591, 0.3183); // (1 / 2pi, 1 / pi)
		uv += 0.5;
		return uv;
	}

	// Maps a 2D uv coordinate to a 3D unit direction vector.
	vec3 EquirectangularToCube(vec2 uv) {
		uv -= 0.5;
		uv *= vec2(TWO_PI, PI);

		// y^2 = sin^2(v)
		// z = sqrt(1 - x*x + y*y));
		// u = atan(z / x);
		// tan(u) = z / x;
		// tan^2(u) = (1 - x*x - y*y) / x*x;

		// x*x * tan^2(u) = 1 - x*x - y*y;
		// x*x * (1 + tan^2(u)) = 1 - y*y;
		// x*x * (sec^2(u)) = 1 - y*y = cos^2(v)
		// x^2 = cos^2(u) * cos^2(v)
		// x = cos(u) * cos(v)
		// z^2 = 1 - cos^2(u)*cos^2(v) - sin^2(v) = cos^2(v) - cos^2(u)*cos^2(v) = sin^2(u) * cos^2(v)
		// z = sin(u) * cos(v)
		return vec3(cos(uv.y) * cos(uv.x), sin(uv.y), cos(uv.y) * sin(uv.x));
	}
}

namespace UnitTest
{
	TEST_CLASS(TestTransform)
	{
	public:
		TEST_METHOD(TestEquirectangular)
		{
			auto fn = [](vec2 uv) {
				vec3 dir = EquirectangularToCube(uv);
				vec2 uv2 = CubeToEquirectangular(dir);
				float dist = dot(uv - uv2, uv - uv2);

				wchar_t msg[512];
				swprintf_s(msg, L"Failed for uv=(%f, %f), dist=%f, diff u=%f v=%f", uv.x, uv.y, dist, uv.x - uv2.x, uv.y - uv2.y);
				Assert::IsTrue(dist <= 0.001f, msg);
			};

			//fn(vec2(0.0, 0.0));
			//fn(vec2(1.0, 1.0));
			fn(vec2(0.01, 0.01));
			fn(vec2(0.99, 0.99));
			fn(vec2(0.5, 0.2));
			fn(vec2(0.3, 0.6));
			fn(vec2(0.8, 0.7));
			fn(vec2(0.2, 0.9));
		}
	};
}
