// Visualize scene depth

#version 460 core

#include "../deferred_common.glsl"
#include "../core/diffuse_sh.glsl"

// Should match with 'r.viewmode'
#define VIEWMODE_SCENEDEPTH  1
#define VIEWMODE_ALBEDO      2
#define VIEWMODE_WORLDNORMAL 3
#define VIEWMODE_METALLIC    4
#define VIEWMODE_ROUGHNESS   5
#define VIEWMODE_EMISSIVE    6
#define VIEWMODE_SSAO        7
#define VIEWMODE_SSR         8
#define VIEWMODE_VELOCITY    9
#define VIEWMODE_CSMLAYER    10
#define VIEWMODE_SKY_SH      11

in VS_OUT {
	vec2 screenUV;
} vs_in;

layout (std140, binding = 1) uniform UBO_VisualizeBuffer {
	int viewmode;
} ubo;

layout (std140, binding = 2) readonly buffer SSBO_SkyDiffuseSH {
	SHBuffer shBuffer;
} ssboSkyDiffuseSH;

layout (binding = 0) uniform sampler2D sceneDepth;
layout (binding = 1) uniform usampler2D gbuf0;
layout (binding = 2) uniform sampler2D gbuf1;
layout (binding = 3) uniform usampler2D gbuf2;
layout (binding = 4) uniform sampler2D ssaoMap;
layout (binding = 5) uniform sampler2D ssr;
layout (binding = 6) uniform sampler2D velocityMap;

layout (location = 0) out vec4 outColor;

void main() {
	vec2 screenUV = vs_in.screenUV;
	int viewmode = ubo.viewmode;
	outColor = vec4(0.5, 0.5, 0.5, 1.0);

	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbuf0, gbuf1, gbuf2, gbufferData);

	if (viewmode == VIEWMODE_SCENEDEPTH) {
		float depth = texture(sceneDepth, screenUV).r;
		float linearDepth = sceneDepthToLinearDepth(vs_in.screenUV, depth);
		outColor = vec4(vec3(linearDepth), 1.0);
	} else if (viewmode == VIEWMODE_ALBEDO) {
		outColor = vec4(gbufferData.albedo, 1.0);
	} else if (viewmode == VIEWMODE_WORLDNORMAL) {
		outColor = vec4((gbufferData.ws_normal + vec3(1.0)) * 0.5, 1.0);
	} else if (viewmode == VIEWMODE_METALLIC) {
		outColor = vec4(vec3(gbufferData.metallic), 1.0);
	} else if (viewmode == VIEWMODE_ROUGHNESS) {
		outColor = vec4(vec3(gbufferData.roughness), 1.0);
	} else if (viewmode == VIEWMODE_EMISSIVE) {
		outColor = vec4(gbufferData.emissive, 1.0);
	} else if (viewmode == VIEWMODE_SSAO) {
		float ssao = texture2D(ssaoMap, screenUV).r;
		outColor = vec4(vec3(ssao), 1.0);
	} else if (viewmode == VIEWMODE_SSR) {
		outColor = vec4(texture2D(ssr, screenUV).rgb, 1.0);
	} else if (viewmode == VIEWMODE_VELOCITY) {
		float fps = 144.0;
		outColor = vec4(abs(texture2D(velocityMap, screenUV).rg) * fps, 0.0, 1.0);
	} else if (viewmode == VIEWMODE_CSMLAYER) {
		DirectionalLight sun = uboPerFrame.sunLight;
		// Green = near, blue = far, red = out of range
		float zNear = uboPerFrame.projParams.x;
		float zFar = sun.shadowMapZFar;
		float linearZ = -gbufferData.vs_coords.z;
		float csmCount = float(sun.shadowMapCascadeCount);
		float csmZSlices[4];
		for (uint i = 0; i < csmCount; ++i) csmZSlices[i] = sun.csmZSlices[i];
		uint csmLayer = 0;
		for (; csmLayer < csmCount; ++csmLayer) {
			if (linearZ < csmZSlices[csmLayer]) break;
		}
		if (gbufferData.material_id != 0) {
			float k = min(1.0, float(csmLayer) / csmCount);
			outColor = vec4(mix(vec3(0, 1, 0), vec3(0, 0, 1), k), 1.0);
		} else {
			outColor = vec4(1.0, 0.0, 0.0, 1.0);
		}
	} else if (viewmode == VIEWMODE_SKY_SH) {
		vec3 dir = gbufferData.ws_normal;
		vec3 color = evaluateSH(ssboSkyDiffuseSH.shBuffer, dir).xyz;
		outColor = vec4(color, 1);
	}
}
