#version 460 core

#include "deferred_common.glsl"

//?#define COPY_MODE
#define COPY_MODE_COLOR      0
#define COPY_MODE_SCENEDEPTH 1

#if COPY_MODE == COPY_MODE_COLOR
	#define OUT_TYPE vec4
#elif COPY_MODE == COPY_MODE_SCENEDEPTH
	#define OUT_TYPE float
#endif

layout (binding = 0) uniform sampler2D sourceTexture;
layout (location = 0) out OUT_TYPE outValue;

in VS_OUT {
	vec2 screenUV;
} fs_in;

void main() {
#if COPY_MODE == COPY_MODE_COLOR
	outValue = texture(sourceTexture, fs_in.screenUV);
#elif COPY_MODE == COPY_MODE_SCENEDEPTH
	float sceneDepth = texture(sourceTexture, fs_in.screenUV).r;
	float linearDepth = sceneDepthToLinearDepth(fs_in.screenUV, sceneDepth);
	outValue = min(65000.0, linearDepth);
#endif
}
