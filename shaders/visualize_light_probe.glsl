// Visualize light probes

#version 460 core

// --------------------------------------------------------
// Common

#include "common.glsl"
#include "deferred_common.glsl"

#if VERTEX_SHADER
	#define INTERPOLANTS_QUALIFIER out
#elif FRAGMENT_SHADER
	#define INTERPOLANTS_QUALIFIER in
#endif

INTERPOLANTS_QUALIFIER Interpolants {
	flat uint instanceID;
	vec3 positionWS;
	vec3 normalWS;
} interpolants;

struct IrradianceProbe {
	vec3 positionWS;
	float captureRadius;
	vec4 uvBounds;
};
struct RadianceProbe {
	vec3 positionWS;
	float captureRadius;
};

layout (std140, binding = 1) uniform UBO_VisualizeLightProbe {
	uint numIrradianceProbes;
	uint numRadianceProbes;
	float irradianceProbeRadius;
	float radianceProbeRadius;
} ubo;

layout (std140, binding = 2) buffer SSBO_0 {
	IrradianceProbe probeArray[];
} ssbo0;

layout (std140, binding = 3) buffer SSBO_1 {
	RadianceProbe probeArray[];
} ssbo1;

// --------------------------------------------------------
// Vertex Shader

#if VERTEX_SHADER

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec3 inNormal;

void main() {
	vec3 center = vec3(0.0);
	float scale = 1.0;
	if (gl_InstanceID < ubo.numIrradianceProbes) {
		center = ssbo0.probeArray[gl_InstanceID].positionWS;
		scale = ubo.irradianceProbeRadius;
	} else {
		center = ssbo1.probeArray[gl_InstanceID - ubo.numIrradianceProbes].positionWS;
		scale = ubo.radianceProbeRadius;
	}

	vec4 positionWS = vec4(inPosition, 1.0);
	positionWS.xyz = positionWS.xyz * scale + center;

	interpolants.instanceID = gl_InstanceID;
	interpolants.positionWS = positionWS.xyz;
	interpolants.normalWS = inNormal;

	gl_Position = uboPerFrame.viewProjTransform * positionWS;
}

#endif // VERTEX_SHADER

// --------------------------------------------------------
// Fragment Shader

#if FRAGMENT_SHADER

layout (binding = 0) uniform sampler2D irradianceAtlas;
layout (binding = 1) uniform samplerCubeArray radianceCubeArray;

layout (location = 0) out vec4 outColor;

void main() {
	vec3 N = normalize(interpolants.normalWS);
	uint instanceID = interpolants.instanceID;

	vec4 debugColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (instanceID < ubo.numIrradianceProbes) {
		vec4 uvBounds = ssbo0.probeArray[instanceID].uvBounds;
		vec2 uv = ONVEncode(N);
		vec2 atlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * uv;
		debugColor.rgb = textureLod(irradianceAtlas, atlasUV, 0).rgb;
	} else {
		uint probeIndex = instanceID - ubo.numIrradianceProbes;
		vec4 R = vec4(N, float(probeIndex));
		debugColor.rgb = textureLod(radianceCubeArray, R, 0).rgb;
	}

	outColor = debugColor;
}

#endif // FRAGMENT_SHADER
