#version 430 core

#include "deferred_common.glsl"

#define APPLY_FOG     1
#define APPLY_SHADOW  1

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright; // bright area only

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform sampler2D gbuf2;

layout (binding = 6) uniform sampler2DShadow sunDepthMap;

uniform vec3 fog_color = vec3(0.7, 0.8, 0.9);

const float PI = 3.14159265359;

// bloom threshold
const float bloom_min = 0.8;
const float bloom_max = 1.2;

struct fragment_info {
	vec3 albedo;
	vec3 normal;
	float specular_power;
	vec3 vs_coords; // in view space
	vec3 ws_coords; // in world space
	uint material_id;
	float metallic;
	float roughness;
	float ao;
};

void unpackGBuffer(ivec2 coord, out fragment_info fragment) {
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	vec4 data2 = texelFetch(gbuf2, coord, 0);
	vec2 temp = unpackHalf2x16(data0.y);

	fragment.albedo = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.normal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	fragment.material_id = data0.w;

	fragment.vs_coords = data1.xyz;
	fragment.ws_coords = vec3(uboPerFrame.inverseViewTransform * vec4(fragment.vs_coords, 1.0));
	fragment.specular_power = data1.w;

	fragment.metallic = data2.x;
	fragment.roughness = data2.y;
	fragment.ao = data2.z;
}

#if APPLY_SHADOW
float getShadowing(fragment_info fragment) {
	// linear depth in view space
	float vz = -fragment.vs_coords.z;
	vz = (vz - uboPerFrame.zRange.x) / (uboPerFrame.zRange.y - uboPerFrame.zRange.x);

	int mapIx = int(vz * 4.0);
	if(mapIx >= 4) {
		return 0.0;
	}

	float u0 = float(mapIx) * 0.25;

	vec4 ls_coords = uboPerFrame.sunViewProjection[mapIx] * vec4(fragment.ws_coords, 1.0);
	ls_coords.xyz = (ls_coords.xyz + vec3(1.0)) * 0.5;
	float NdotL = dot(fragment.normal, -uboPerFrame.dirLightDirs[0]);
	float bias = max(0.05 * (1.0 - NdotL), 0.005);
	float inv_w = 1.0 / ls_coords.w;
	ls_coords.xy = ls_coords.xy * inv_w;
	ls_coords.z = (ls_coords.z - bias) * inv_w;

	vec2 dudv = 1.0 / vec2(textureSize(sunDepthMap, 0));
	dudv.x *= 0.25;

	float shadow = 0.0f;
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
			vec3 uvw = ls_coords.xyz;
			uvw.x = u0 + 0.25 * uvw.x;
			uvw.xy += dudv * vec2(x, y);
			shadow += texture(sunDepthMap, uvw);
		}
	}
	shadow /= 9.0;
	
	return max(0.5, shadow);
}
#endif

vec3 phongShading(fragment_info fragment) {
	vec3 result = vec3(0.0);
	vec3 N = fragment.normal;
	for(uint i = 0; i < uboPerFrame.numDirLights; ++i) {
		vec3 L = -uboPerFrame.dirLightDirs[i];
		float cosTheta = max(0.0, dot(N, L));
		vec3 diffuse_color = uboPerFrame.dirLightColors[i] * fragment.albedo * cosTheta;
		result += diffuse_color;
	}
	for(uint i = 0; i < uboPerFrame.numPointLights; ++i) {
		vec3 L = uboPerFrame.pointLightPos[i] - fragment.vs_coords;
		float dist = length(L);
		float attenuation = 500.0 / (pow(dist, 2.0) + 1.0);
		L = normalize(L);
		vec3 R = reflect(-L, N);
		float cosTheta = max(0.0, dot(N, L));
		vec3 specular_color = uboPerFrame.pointLightColors[i] * pow(max(0.0, dot(R, -uboPerFrame.eyeDirection)), fragment.specular_power);
		vec3 diffuse_color = uboPerFrame.pointLightColors[i] * fragment.albedo * cosTheta;
		result += attenuation * (diffuse_color + specular_color);
	}

#if APPLY_SHADOW
	result.rgb = result.rgb * getShadowing(fragment);
#endif

	return result;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 pbrShading(fragment_info fragment) {
	vec3 N = fragment.normal;
	//N.y = -N.y;
	vec3 V = normalize(uboPerFrame.eyePosition - fragment.vs_coords);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, fragment.albedo, fragment.metallic);

	vec3 Lo = vec3(0.0);

	for(int i=0; i<uboPerFrame.numDirLights; ++i) {
		vec3 L = -uboPerFrame.dirLightDirs[i];
		vec3 H = normalize(V + L);
		vec3 radiance = uboPerFrame.dirLightColors[i];

		float NDF = distributionGGX(N, H, fragment.roughness);
		float G = geometrySmith(N, V, L, fragment.roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - fragment.metallic;

		vec3 num = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = num / max(denom, 0.001);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * fragment.albedo / PI + specular) * radiance * NdotL;
	}

	for(int i=0; i<uboPerFrame.numPointLights; ++i) {
		vec3 L = normalize(uboPerFrame.pointLightPos[i] - fragment.vs_coords);
		vec3 H = normalize(V + L);
		float distance = length(uboPerFrame.pointLightPos[i] - fragment.vs_coords);
		float attenuation = 1000.0 / (1000.0 + distance * distance);
		vec3 radiance = uboPerFrame.pointLightColors[i];
		radiance *= attenuation;

		float NDF = distributionGGX(N, H, fragment.roughness);
		float G = geometrySmith(N, V, L, fragment.roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - fragment.metallic;

		vec3 num = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = num / max(denom, 0.001);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * fragment.albedo / PI + specular) * radiance * NdotL;
	}

	vec3 ambient = vec3(0.03) * fragment.albedo * fragment.ao;
	vec3 color = ambient + Lo;

#if APPLY_SHADOW
	color = color * getShadowing(fragment);
#endif

	return color;
}

vec4 calculateShading(fragment_info fragment) {
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if(fragment.material_id == MATERIAL_ID_SOLID_COLOR || fragment.material_id == MATERIAL_ID_TEXTURE) {
		result.rgb += phongShading(fragment);
	} else if(fragment.material_id == MATERIAL_ID_WIREFRAME) {
		result.rgb += fragment.albedo;
	} else if(fragment.material_id == MATERIAL_ID_PBR) {
		result.rgb += pbrShading(fragment);
	} else {
		discard;
	}
	return result;
}

#if APPLY_FOG
vec3 applyFog(fragment_info fragment, vec3 color) {
	const float magic_number = 0.05; // proper physical meaning?
	float z = length(fragment.vs_coords) * magic_number;
	float de = 0.025 * smoothstep(0.0, 6.0, 10.0 - fragment.ws_coords.y);
	float di = 0.045 * smoothstep(0.0, 40.0, 20.0 - fragment.ws_coords.y);
	float extinction = exp(-z * de);
	float inscattering = exp(-z * di);
	return color * extinction + fog_color * (1.0 - inscattering);
}
#endif

void main() {
	fragment_info fragment;
	unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);
	vec4 color = calculateShading(fragment);

#if APPLY_FOG
	color.rgb = applyFog(fragment, color.rgb);
#endif

	// output: standard shading
	out_color = color;

	// for depth-of-field. continue to blur_pass.glsl
	out_color.a = -fragment.vs_coords.z;

	// output: light bloom
	float Y = dot(color.xyz, vec3(0.299, 0.587, 0.144));
	color.xyz = color.xyz * 4.0 * smoothstep(bloom_min, bloom_max, Y);
	out_bright = color;
}