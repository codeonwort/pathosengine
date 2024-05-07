//? #version 460 core
//? #include "deferred_common.glsl"

/* --------------------------------------------------------------

CAUTION: Need to include deferred_common.glsl prior to this file

-------------------------------------------------------------- */

#define DEBUG_CSM_ID     0

#define SOFT_SHADOW      1
#define MAX_CASCADES     4
#define MIN_SHADOWING    0.1

struct ShadowQuery {
	vec3 vPos;    // position in view space
	float zFar;   // Probably 'r.csm.zFar'
	vec3 wPos;    // position in world space
	vec3 vNormal; // surface normal in view space
	vec3 wNormal; // surface normal in world space
};

struct OmniShadowQuery {
	int  shadowMapIndex;
	vec3 lightPos;
	float attenuationRadius;
	vec3 wPos;
};

float getShadowingFactor(sampler2DArrayShadow csm, ShadowQuery query) {
	vec3 vSun = uboPerFrame.sunLight.vsDirection;
	float zNear = uboPerFrame.zRange.x; // zNear is same as that of primary view. (no reason, just same)
	//float linearZ = (-query.vPos.z - zNear) / (query.zFar - zNear);
	float linearZ = -query.vPos.z;

	// Make array here due to stupid uniform alignment
	uint csmCount = uboPerFrame.csmCount;
	float csmDepths[MAX_CASCADES];
	for (uint i = 0; i < uboPerFrame.csmCount; ++i) {
		csmDepths[i] = uboPerFrame.csmDepths[i];
	}
	
	uint csmLayer = 0;
	for (; csmLayer < csmCount; ++csmLayer) {
		if (linearZ < csmDepths[csmLayer]) {
			break;
		}
	}
	if (csmLayer >= csmCount) {
		return 1.0;
	}

#if DEBUG_CSM_ID
	return float(csmLayer) / csmCount;
#endif

	const float SLOPE_BIAS = 0.005;
	const float NORMAL_OFFSET = 0.01; // #todo-glsl: Magic value for world_rc1, but too big for world1.

	// Convert world query position to the shadow texture coordinate
	vec4 ls_coords = uboPerFrame.sunViewProjection[csmLayer] * vec4(query.wPos + NORMAL_OFFSET * query.wNormal, 1.0);
	float inv_w = 1.0 / ls_coords.w;
	ls_coords.xyz = ls_coords.xyz * inv_w;
	ls_coords.xyz = (ls_coords.xyz + vec3(1.0)) * 0.5;

	// Apply slope-scaled depth bias
	float NdotL = max(dot(query.vNormal, -vSun), 0.0);
	float bias = max(SLOPE_BIAS * (1.0 - NdotL), SLOPE_BIAS);
	//float bias = SLOPE_BIAS * tan(acos(NdotL));
	ls_coords.z -= clamp(bias, 0.0, SLOPE_BIAS);

	float shadow = 0.0;

#if SOFT_SHADOW
	vec2 dudv = 1.0 / vec2(textureSize(csm, 0));
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
			vec4 shadowSamplePos = vec4(ls_coords.xy, float(csmLayer), ls_coords.z);
			shadowSamplePos.xy += dudv * vec2(x, y);
			shadow += texture(csm, shadowSamplePos);
		}
	}
	shadow /= 9.0;
#else
	vec4 shadowSamplePos = vec4(ls_coords.xy, float(csmLayer), ls_coords.z);
	shadow = texture(csm, shadowSamplePos);
#endif
	
	return max(MIN_SHADOWING, shadow);
}

float getOmniShadowingFactor(samplerCubeArrayShadow shadowMaps, OmniShadowQuery query) {
	const float DEPTH_BIAS = 0.01;
	float Z_FAR = query.attenuationRadius;

	vec3 fragToLight = query.wPos - query.lightPos;

	float currentDepth = length(fragToLight);
	currentDepth = max(0.0, currentDepth / Z_FAR - DEPTH_BIAS);

	// #todo: Soft shadow
	vec4 P = vec4(fragToLight, float(query.shadowMapIndex));
	float shadow = texture(shadowMaps, P, currentDepth - DEPTH_BIAS);

	return shadow;
};
