#version 460 core

#include "deferred_common.glsl"

layout (binding = 0) uniform sampler2D inputTexture; // sceneColor
layout (binding = 1) uniform usampler2D gbufferA;
layout (binding = 2) uniform sampler2D gbufferB;
layout (binding = 3) uniform usampler2D gbufferC;

layout (location = 0) out vec4 outputColor; // sceneColorDownsample

layout (std140, binding = 1) uniform UBO_BloomSetup {
	vec2 sceneSize;
	uint applyThreshold;
	float bloomThreshold;
	float exposureScale;
} ubo;

in VS_OUT {
	vec2 screenUV;
} fs_in;

// #todo-glsl: Parametrize
const float preExposure = 1.0;
const float preExposureInv = 1.0;

float getLuminance(vec3 v) {
	return dot(v, vec3(0.3, 0.59, 0.11));
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(fs_in.screenUV * ubo.sceneSize), gbufferA, gbufferB, gbufferC, gbufferData);

	vec4 sceneColor = texture(inputTexture, fs_in.screenUV);

	vec3 linearColor = sceneColor.rgb;
	if (ubo.applyThreshold != 0) {
		linearColor = max(vec3(0.0), linearColor - ubo.bloomThreshold);
	}
	linearColor += gbufferData.emissive;

	// Limit to 65536.0
	linearColor = min(vec3(256.0 * 256.0), linearColor);

	float bloomLuminance = getLuminance(linearColor) * ubo.exposureScale;
	float bloomAmount = clamp(bloomLuminance * 0.5, 0.0, 1.0);

	outputColor = vec4(bloomAmount * linearColor, 0.0) * preExposure;
}
