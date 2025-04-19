#include "pch.h"
#include "CppUnitTest.h"

#include "pathos/loader/image_loader.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/file_system.h"
#include <array>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pathos;

// "An Efficient Representation for Irradiance Environment Maps" by Ravi Ramamoorthi and Pat Hanrahan.
namespace sh {
	const float PI = 3.14159265359f;
	const float TWO_PI = 6.28318530718f;
	const float HALF_PI = 1.57079632679489661923f;

	struct Skybox {
		bool bIsConstant;
		vector3 constantColor;
		int32 cubemapSize;
		std::vector<ImageBlob*> imageData;

		Skybox(const vector3& inConstantColor, uint32 inCubemapSize) {
			bIsConstant = true;
			constantColor = inConstantColor;
			cubemapSize = inCubemapSize;
		}
		Skybox(const std::vector<ImageBlob*>& inImageData) {
			bIsConstant = false;
			constantColor = vector3(0.0f);
			imageData = inImageData;
			cubemapSize = imageData[0]->width;
		}

		int32 getCubemapSize() const { return cubemapSize; }

		//vector3 sample(const vector3& dir) const {
		//	return 10.0f * (0.5f * (dir + vector3(1.0f)));
		//}
		vector3 texelFetch(int32 face, int32 x, int32 y) const {
			if (bIsConstant) return constantColor;

			ImageBlob* blob = imageData[face];
			const uint8* bytes = blob->rawBytes;
			bytes += (blob->bpp / 8) * ((y * blob->width) + x);
			uint8 r = bytes[0];
			uint8 g = bytes[1];
			uint8 b = bytes[2];
			return vector3(r, g, b) / 255.0f;
		};
	};

	struct SHBuffer {
		std::vector<vector3> Ls;
	};

	SHBuffer prefilter(const Skybox& skybox) {
		SHBuffer buf;

		auto Y00 = [](const vector3& dir) { return 0.282095f; };
		auto Y1_1 = [](const vector3& dir) { return 0.488603f * dir.y; };
		auto Y10 = [](const vector3& dir) { return 0.488603f * dir.z; };
		auto Y11 = [](const vector3& dir) { return 0.488603f * dir.x; };
		auto Y2_2 = [](const vector3& dir) { return 1.092548f * dir.x * dir.y; };
		auto Y2_1 = [](const vector3& dir) { return 1.092548f * dir.y * dir.z; };
		auto Y20 = [](const vector3& dir) { return 0.315392f * (3.0f * dir.z * dir.z - 1.0f); };
		auto Y21 = [](const vector3& dir) { return 1.092548f * dir.x * dir.z; };
		auto Y22 = [](const vector3& dir) { return 0.546274f * (dir.x * dir.x - dir.y * dir.y); };
		std::function<float(const vector3&)> Ys[] = { Y00, Y1_1, Y10, Y11, Y2_2, Y2_1, Y20, Y21, Y22 };

		for (uint32 i = 0; i < 9; ++i) {
			vector3 L(0.0f);
			uint32 nrSamples = 0;
			float wSum = 0.0f;
			for (int32 face = 0; face < 6; ++face) {
				const int32 width = skybox.getCubemapSize();
				const int32 height = skybox.getCubemapSize();
				for (int32 y = 0; y < height; ++y) {
					for (int32 x = 0; x < width; ++x) {
						const float u = 2.0f * ((float)x / width) - 1.0f;
						const float v = 2.0f * ((float)y / height) - 1.0f;

						const float tmp = 1.0f + u * u + v * v;
						const float w = 4.0f / (tmp * sqrtf(tmp));

						vector3 dir;
						if (face == 0) dir = normalize(vector3(1, u, v));
						else if (face == 1) dir = normalize(vector3(-1, u, v));
						else if (face == 2) dir = normalize(vector3(u, 1, v));
						else if (face == 3) dir = normalize(vector3(u, -1, v));
						else if (face == 4) dir = normalize(vector3(u, v, 1));
						else if (face == 5) dir = normalize(vector3(u, v, -1));

						float cosTheta = dir.z;
						float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

						L += w * skybox.texelFetch(face, x, y) * Ys[i](dir);
						wSum += w;
						nrSamples++;
					}
				}
			}
			L *= 4.0f * sh::PI / wSum;
			buf.Ls.push_back(L);
		}

		return buf;
	}

	vector3 evaluate(const SHBuffer& shBuffer, float x, float y, float z) {
		const float c1 = 0.429043f;
		const float c2 = 0.511664f;
		const float c3 = 0.743125f;
		const float c4 = 0.886227f;
		const float c5 = 0.247708f;

		const vector3& L00 = shBuffer.Ls[0];
		const vector3& L1_1 = shBuffer.Ls[1];
		const vector3& L10 = shBuffer.Ls[2];
		const vector3& L11 = shBuffer.Ls[3];
		const vector3& L2_2 = shBuffer.Ls[4];
		const vector3& L2_1 = shBuffer.Ls[5];
		const vector3& L20 = shBuffer.Ls[6];
		const vector3& L21 = shBuffer.Ls[7];
		const vector3& L22 = shBuffer.Ls[8];

		vector3 E(0.0f);
		E += c1 * L22 * (x * x - y * y) + c3 * L20 * z * z + c4 * L00 - c5 * L20;
		E += 2 * c1 * (L2_2 * x * y + L21 * x * z + L2_1 * y * z);
		E += 2 * c2 * (L11 * x + L1_1 * y + L10 * z);
		return E;
	}
}

namespace UnitTest
{
	TEST_CLASS(TestIrradianceMap)
	{
	public:
		TEST_METHOD(FurnaceTest) {
			const vector3 SKY_COLOR(0.18f);

			sh::Skybox skybox{ SKY_COLOR, 128 };
			sh::SHBuffer shBuffer = sh::prefilter(skybox);

			uint32 numInvalid = 0;
			const float eps = 0.03f; // 3 percent

			const int32 width = 128;
			const int32 height = 128;
			for (uint32 face = 0; face < 6; ++face) {
				std::vector<uint8> imageData(width * height * 3);
				size_t p = 0;
				for (int32 y = 0; y < height; ++y) {
					for (int32 x = 0; x < width; ++x) {
						const float u = 2.0f * ((float)x / width) - 1.0f;
						const float v = 2.0f * ((float)y / height) - 1.0f;

						vector3 dir;
						if (face == 0) dir = normalize(vector3(u, v, 1));
						else if (face == 1) dir = normalize(vector3(u, v, -1));
						else if (face == 2) dir = normalize(vector3(1, u, v));
						else if (face == 3) dir = normalize(vector3(-1, u, v));
						else if (face == 4) dir = normalize(vector3(u, 1, v));
						else if (face == 5) dir = normalize(vector3(u, -1, v));

						vector3 color = evaluate(shBuffer, dir.x, dir.y, dir.z); // irradiance
						float ratio = color.x / SKY_COLOR.x;
						if (std::abs(ratio - sh::PI) > eps) {
							float diff = std::abs(ratio - sh::PI);
							numInvalid += 1;
						}

						imageData[p + 0] = std::min(0xFF, std::max(0, int32(color.r * 255.0f)));
						imageData[p + 1] = std::min(0xFF, std::max(0, int32(color.g * 255.0f)));
						imageData[p + 2] = std::min(0xFF, std::max(0, int32(color.b * 255.0f)));
						p += 3;
					}
				}
			}

			Assert::IsTrue(numInvalid == 0, L"Furnace test has failed");
		}

		TEST_METHOD(SkyboxImage)
		{
			ResourceFinder::get().add("../");
			ResourceFinder::get().add("../../");
			ResourceFinder::get().add("../../shaders/");
			ResourceFinder::get().add("../../resources/");
			ResourceFinder::get().add("../../resources_external/");

			std::array<const char*, 6> cubeImgName = {
				"resources/skybox/cubemap1/pos_x.jpg",
				"resources/skybox/cubemap1/neg_x.jpg",
				"resources/skybox/cubemap1/pos_y.jpg",
				"resources/skybox/cubemap1/neg_y.jpg",
				"resources/skybox/cubemap1/pos_z.jpg",
				"resources/skybox/cubemap1/neg_z.jpg"
			};
			auto rescale = ImageUtils::RescaleDesc::downscaleOnly(256, 256, ImageUtils::ERescaleFilter::Bilinear);
			//auto rescale = ImageUtils::RescaleDesc::noScale();
			auto cubeImages = ImageUtils::loadCubemapImages(cubeImgName, ECubemapImagePreference::HLSL, rescale);

			sh::Skybox skybox{ cubeImages };
			sh::SHBuffer shBuffer = sh::prefilter(skybox);

			std::string screenshotDir = pathos::getSolutionDir() + "/log/test_dump/";
			pathos::createDirectory(screenshotDir.c_str());

			const int32 width = 128;
			const int32 height = 128;
			for (uint32 face = 0; face < 6; ++face) {
				std::vector<uint8> imageData(width * height * 3);
				size_t p = 0;
				for (int32 y = 0; y < height; ++y) {
					for (int32 x = 0; x < width; ++x) {
						const float u = 2.0f * ((float)x / width) - 1.0f;
						const float v = 2.0f * ((float)y / height) - 1.0f;

						vector3 dir;
						if (face == 0) dir = normalize(vector3(1, u, v));
						else if (face == 1) dir = normalize(vector3(-1, u, v));
						else if (face == 2) dir = normalize(vector3(u, 1, v));
						else if (face == 3) dir = normalize(vector3(u, -1, v));
						else if (face == 4) dir = normalize(vector3(u, v, 1));
						else if (face == 5) dir = normalize(vector3(u, v, -1));

						// Too bright to output as SDR image, let's divide by PI.
						vector3 color = evaluate(shBuffer, dir.x, dir.y, dir.z) / sh::PI;

						imageData[p + 0] = std::min(0xFF, std::max(0, int32(color.r * 255.0f)));
						imageData[p + 1] = std::min(0xFF, std::max(0, int32(color.g * 255.0f)));
						imageData[p + 2] = std::min(0xFF, std::max(0, int32(color.b * 255.0f)));
						p += 3;
					}
				}

				// I don't have reference data to compare. At least let's output the result as image.
				std::string screenshotPath = screenshotDir;
				screenshotPath += "sh_output_";
				screenshotPath += std::to_string(face);
				screenshotPath += ".png";
				ImageUtils::saveRGB8ImageAsPNG(width, height, imageData.data(), screenshotPath.data());
			}
		}
	};
}
