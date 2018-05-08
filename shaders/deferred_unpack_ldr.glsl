#version 430 core

// should match with MeshDeferredRenderPass_Unpack::MAX_DIRECTIONAL_LIGHTS
#define MAX_DIRECTIONAL_LIGHTS 8

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;

// in view space
uniform vec3 eyeDirection;

uniform uint numDirLights;
uniform vec3 dirLightDirs[MAX_DIRECTIONAL_LIGHTS];
uniform vec3 dirLightColors[MAX_DIRECTIONAL_LIGHTS];

uniform uint numPointLights;
uniform vec3 pointLightPos[16];
uniform vec3 pointLightColors[16];

struct fragment_info {
	vec3 color;
	vec3 normal;
	float specular_power;
	vec3 ws_coords;
	uint material_id;
};

void unpackGBuffer(ivec2 coord, out fragment_info fragment) {
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	vec2 temp = unpackHalf2x16(data0.y);

	fragment.color = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.normal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	fragment.material_id = data0.w;

	fragment.ws_coords = data1.xyz;
	fragment.specular_power = data1.w;
}

vec4 calculateShading(fragment_info fragment) {
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if(fragment.material_id != 0){
		vec3 N = fragment.normal;
		for(uint i = 0; i < numDirLights; ++i) {
			vec3 L = -dirLightDirs[i];
			float cosTheta = max(0.0, dot(N, L));
			vec3 diffuse_color = dirLightColors[i] * fragment.color * cosTheta;
			result += vec4(diffuse_color, 0.0);
		}
		for(uint i = 0; i < numPointLights; ++i) {
			vec3 L = pointLightPos[i] - fragment.ws_coords;
			float dist = length(L);
			float attenuation = 500.0 / (pow(dist, 2.0) + 1.0);
			L = normalize(L);
			vec3 R = reflect(-L, N);
			float cosTheta = max(0.0, dot(N, L));
			vec3 specular_color = pointLightColors[i] * pow(max(0.0, dot(R, -eyeDirection)), fragment.specular_power);
			vec3 diffuse_color = pointLightColors[i] * fragment.color * cosTheta;
			result += vec4(attenuation * (diffuse_color + specular_color), 0.0);
		}
	}else discard;
	return result;
}

void main() {
	fragment_info fragment;
	unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);
	out_color = calculateShading(fragment);
}