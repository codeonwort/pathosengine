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

	vec4 prevClipPos = uboPerFrame.prevViewProjTransform * vec4(currWorldPos, 1.0);
	vec2 prevScreenUV = 0.5 * (prevClipPos.xy / prevClipPos.w) + vec2(0.5, 0.5);

	vec4 prevColor = currColor;
	if (all(lessThanEqual(vec2(0.0, 0.0), prevScreenUV))
		&& all(lessThan(prevScreenUV, vec2(1.0, 1.0))))
	{
		prevColor = texture(inSceneColorHistory, prevScreenUV);
	}

	outSceneColor = (1.0 - w) * currColor + w * prevColor;
}
