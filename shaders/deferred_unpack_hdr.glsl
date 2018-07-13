#version 430 core

// should match with MeshDeferredRenderPass_Unpack::MAX_DIRECTIONAL_LIGHTS
#define MAX_DIRECTIONAL_LIGHTS 8
#define MAX_POINT_LIGHTS 16

#define APPLY_FOG 0

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright; // bright area only

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform sampler2D gbuf2;

// in view space
layout (std140, binding = 0) uniform UBO_UnpackHDR {
	vec3 eyeDirection;
	vec3 eyePosition;

	uint numDirLights;
	vec3 dirLightDirs[MAX_DIRECTIONAL_LIGHTS];
	vec3 dirLightColors[MAX_DIRECTIONAL_LIGHTS];

	uint numPointLights;
	vec3 pointLightPos[MAX_POINT_LIGHTS];
	vec3 pointLightColors[MAX_POINT_LIGHTS];
} ubo;

uniform vec3 fog_color = vec3(0.7, 0.8, 0.9);

const float PI = 3.14159265359;

// bloom threshold
const float bloom_min = 0.8;
const float bloom_max = 1.2;

struct fragment_info {
	vec3 albedo;
	vec3 normal;
	float specular_power;
	vec3 ws_coords; // in view space
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

	fragment.ws_coords = data1.xyz;
	fragment.specular_power = data1.w;

	fragment.metallic = data2.x;
	fragment.roughness = data2.y;
	fragment.ao = data2.z;
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
	vec3 V = normalize(ubo.eyePosition - fragment.ws_coords);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, fragment.albedo, fragment.metallic);

	vec3 Lo = vec3(0.0);

	for(int i=0; i<ubo.numDirLights; ++i) {
		vec3 L = -ubo.dirLightDirs[i];
		vec3 H = normalize(V + L);
		vec3 radiance = ubo.dirLightColors[i];

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

	for(int i=0; i<ubo.numPointLights; ++i) {
		vec3 L = normalize(ubo.pointLightPos[i] - fragment.ws_coords);
		vec3 H = normalize(V + L);
		float distance = length(ubo.pointLightPos[i] - fragment.ws_coords);
		float attenuation = 1000.0 / (1000.0 + distance * distance);
		vec3 radiance = ubo.pointLightColors[i];
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

	return color;
}

vec4 calculateShading(fragment_info fragment) {
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if(fragment.material_id == 1 || fragment.material_id == 3) {
		// old shading
		vec3 N = fragment.normal;
		for(uint i = 0; i < ubo.numDirLights; ++i) {
			vec3 L = -ubo.dirLightDirs[i];
			float cosTheta = max(0.0, dot(N, L));
			vec3 diffuse_color = ubo.dirLightColors[i] * fragment.albedo * cosTheta;
			result += vec4(diffuse_color, 0.0);
		}
		for(uint i = 0; i < ubo.numPointLights; ++i) {
			vec3 L = ubo.pointLightPos[i] - fragment.ws_coords;
			float dist = length(L);
			float attenuation = 500.0 / (pow(dist, 2.0) + 1.0);
			L = normalize(L);
			vec3 R = reflect(-L, N);
			float cosTheta = max(0.0, dot(N, L));
			vec3 specular_color = ubo.pointLightColors[i] * pow(max(0.0, dot(R, -ubo.eyeDirection)), fragment.specular_power);
			vec3 diffuse_color = ubo.pointLightColors[i] * fragment.albedo * cosTheta;
			result += vec4(attenuation * (diffuse_color + specular_color), 0.0);
		}
	} else if(fragment.material_id == 8) {
		result.rgb += pbrShading(fragment);
	} else discard;
	return result;
}

#if APPLY_FOG
vec3 applyFog(fragment_info fragment, vec3 color) {
	float z = length(fragment.ws_coords);
	// WRONG!!!
	// I need y values in world space, not view space...
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
	out_color.a = -fragment.ws_coords.z;

	// output: light bloom
	float Y = dot(color.xyz, vec3(0.299, 0.587, 0.144));
	color.xyz = color.xyz * 4.0 * smoothstep(bloom_min, bloom_max, Y);
	out_bright = color;
}