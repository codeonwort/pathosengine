#version 460 core

#include "common.glsl"
#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform usampler2D gbuf2;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbuf0, gbuf1, gbuf2, gbufferData);

	if (gbufferData.material_id == MATERIAL_SHADINGMODEL_UNLIT) {
		outSceneColor = vec4(gbufferData.albedo, 0.0);
	} else if (gbufferData.material_id == MATERIAL_SHADINGMODEL_DEFAULTLIT) {
		// This pass was originally written only for unlit,
		// but it turned out that it's actually a good place to add emissive to sceneColor.
		outSceneColor = vec4(gbufferData.emissive, 0.0);
	} else {
		discard;
	}
}
