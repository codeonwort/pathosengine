#version 460 core

#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_TAA {
	float historyWeight;
} ubo;

layout (binding = 0) uniform sampler2D inSceneColor;
layout (binding = 1) uniform sampler2D inSceneColorHistory;
layout (binding = 2) uniform sampler2D inSceneDepth;
layout (binding = 3) uniform sampler2D inVelocityMap;
layout (binding = 4) uniform usampler2D inGBufferA;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

void main() {
	vec2 screenUV = fs_in.screenUV;
	float w = ubo.historyWeight;

	vec4 currColor = texture(inSceneColor, screenUV);
	float currSceneDepth = texture(inSceneDepth, screenUV).r;
	vec3 currWorldPos = getWorldPositionFromSceneDepth(screenUV, currSceneDepth);

	// #todo-taa: Sample velocityMap only for dynamic region.
#if 0
	vec4 prevClipPos = uboPerFrame.prevViewProjTransform * vec4(currWorldPos, 1.0);
	vec2 prevScreenUV = 0.5 * (prevClipPos.xy / prevClipPos.w) + vec2(0.5, 0.5);
#else
	vec2 velocity = texture(inVelocityMap, screenUV).rg;
	vec2 prevScreenUV = screenUV - velocity;
#endif

	vec4 prevColor = currColor;
	if (all(lessThanEqual(vec2(0.0, 0.0), prevScreenUV))
		&& all(lessThan(prevScreenUV, vec2(1.0, 1.0))))
	{
		// Neighborhood clamping
		float dx = uboPerFrame.screenResolution.z;
		float dy = uboPerFrame.screenResolution.w;
		vec4 neighborMin = currColor;
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(-dx, -dy)));
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(0, -dy)));
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(dx, -dy)));
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(-dx, 0)));
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(dx, 0)));
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(-dx, dy)));
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(0, dy)));
		neighborMin = min(neighborMin, texture(inSceneColor, screenUV + vec2(dx, dy)));
		vec4 neighborMax = currColor;
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(-dx, -dy)));
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(0, -dy)));
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(dx, -dy)));
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(-dx, 0)));
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(dx, 0)));
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(-dx, dy)));
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(0, dy)));
		neighborMax = max(neighborMax, texture(inSceneColor, screenUV + vec2(dx, dy)));

		prevColor = texture(inSceneColorHistory, prevScreenUV);
		prevColor = clamp(prevColor, neighborMin, neighborMax);

		uint shadingModelID = texture(inGBufferA, screenUV).w;
		// Probably sky, which does not generate motion vector.
		// Even if its motion vector is valid, it's unlikely it will benefit from subpixel details,
		// as sky atmosphere is analytical and sky cubemaps do not have spatial artifacts.
		if (shadingModelID == 0) {
			w = 0.0;
		}
	}

	outSceneColor = (1.0 - w) * currColor + w * prevColor;
}
