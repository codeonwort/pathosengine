// Visualize scene depth

#version 430 core

#include "deferred_common.glsl"

layout (binding = 0) uniform sampler2D sceneDepth;
layout (location = 0) out vec4 color;

in VS_OUT {
	vec2 screenUV;
} vs_in;

void main() {
	float depth = texture(sceneDepth, vs_in.screenUV).r;
	float linearDepth = sceneDepthToLinearDepth(vs_in.screenUV, depth);
	color = vec4(vec3(linearDepth), 1.0);
}
