#include "random.h"
#include "glm/gtc/constants.hpp"

glm::vec3 RandomInUnitSphere()
{
	static thread_local RNG randoms(4096);

	// PBR Ch. 13
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float z = 1.0f - 2.0f * u1;
	float r = sqrt(std::max(0.0f, 1.0f - z * z));
	float phi = 2.0f * glm::pi<float>() * u2;
	return glm::vec3(r * cos(phi), r * sin(phi), z);
}

float Random()
{
	static thread_local RNG randoms(4096 * 8);

	return randoms.Peek();
}

glm::vec3 RandomInUnitDisk()
{
	static thread_local RNG randoms(4096 * 8);
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float r = sqrt(u1);
	
	float theta = 2.0f * glm::pi<float>() * u2;
	return glm::vec3(r * cos(theta), r * sin(theta), 0.0f);
}
