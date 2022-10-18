#include "random.h"
#include "glm/gtc/constants.hpp"

vector3 RandomInUnitSphere()
{
	static thread_local RNG randoms(4096);

	// PBR Ch. 13
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float z = 1.0f - 2.0f * u1;
	float r = sqrt(std::max(0.0f, 1.0f - z * z));
	float phi = 2.0f * glm::pi<float>() * u2;
	return vector3(r * cos(phi), r * sin(phi), z);
}

float Random()
{
	static thread_local RNG randoms(4096 * 8);

	return randoms.Peek();
}

vector3 RandomInUnitDisk()
{
	static thread_local RNG randoms(4096 * 8);
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float r = sqrt(u1);
	
	float theta = 2.0f * glm::pi<float>() * u2;
	return vector3(r * cos(theta), r * sin(theta), 0.0f);
}

namespace badger {

	// https://en.wikipedia.org/wiki/Halton_sequence
	void HaltonSequence(uint32 base, uint32 numSamples, float* dest)
	{
		int32 n = 0, d = 1;
		for (uint32 step = 0; step < numSamples; ++step) {
			int32 x = d - n;
			if (x == 1) {
				n = 1;
				d *= base;
			} else {
				int32 y = d / base;
				while (x <= y) {
					y /= base;
				}
				n = (base + 1) * y - x;
			}
			dest[step] = (float)n / d;
		}
	}

}
