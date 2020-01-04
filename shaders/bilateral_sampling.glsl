#version 450 core

// Bilateral filter: https://www.shadertoy.com/view/4dfGDH

#include "deferred_common.glsl"

#define BSIGMA 10.0
#define MSIZE 15

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0, rgba32f) readonly uniform image2D sourceImage;
layout (binding = 1, rgba32f) writeonly uniform image2D targetImage;

layout (std140, binding = 1) uniform UBO_SSAO {
	vec4 kernel[4];
	float bZ; float _pad[3];
} ubo;

float normpdf3(in vec3 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

float getKernel(int ix) {
	return ubo.kernel[ix / 4][ix % 4];
}

void main() {
	ivec2 sceneSize = imageSize(sourceImage);
	if (gl_GlobalInvocationID.x >= sceneSize.x) {
		return;
	}

	ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);

	const int kSize = (MSIZE - 1) / 2;
	vec3 final_colour = vec3(0.0);
	
	vec3 c = imageLoad(sourceImage, currentTexel).rgb;
	vec3 cc;
	float factor;
	float Z = 0.0;

	// Read out the texels
	for (int i = -kSize; i <= kSize; ++i)
	{
		for (int j = -kSize; j <= kSize; ++j)
		{
			cc = imageLoad(sourceImage, currentTexel + ivec2(i, j)).rgb;
			factor = normpdf3(cc - c, BSIGMA) * ubo.bZ * getKernel(kSize + j) * getKernel(kSize + i);
			Z += factor;
			final_colour += factor * cc;
		}
	}
	final_colour /= Z;

	imageStore(targetImage, currentTexel, vec4(final_colour, 0.0));
}
