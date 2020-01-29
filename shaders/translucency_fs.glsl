#version 430 core

#include "brdf.glsl"
#include "deferred_common.glsl"

layout (location = 0) out vec4 output0; // (color, opacity)

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	mat3 mvTransform3x3;
	vec4 albedo;
	vec4 metallic_roughness;
	vec4 transmittance_opacity;
} uboPerObject;

in VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
    vec3 bitangent;
	vec2 texcoord;
} fs_in;

void main() {
	//vec3 albedo        = uboPerObject.albedo.rgb;
	//float metallic     = uboPerObject.metallic_roughness.x;
	//float roughness    = uboPerObject.metallic_roughness.y;
	vec3 transmittance = uboPerObject.transmittance_opacity.xyz;
	//float opacity      = uboPerObject.transmittance_opacity.w;

	vec3 vs_coords = fs_in.vs_coords;

	vec3 N = normalize(fs_in.normal);
	vec3 L = -uboPerFrame.directionalLights[0].direction;
	vec3 V = normalize(uboPerFrame.eyePosition - vs_coords);
	vec3 H = normalize(V + L);

	// Fake glass material
	vec3 albedo = vec3(0.1, 0.1, 0.2);
	float fakeFresnel = fresnelSchlick(max(dot(H, V), 0.0), vec3(3.0)).x;
	float metallic = mix(0.05, 1.0, fakeFresnel);
	float roughness = 0.0;
	float opacity = mix(0.05, 0.2, fakeFresnel);
	float refraction = mix(1.2, 0.8, fakeFresnel);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(albedo, vec3(1.0)), metallic);

	vec3 Lo = vec3(0.0);
	
	// Directional lights
	for (int i = 0; i < uboPerFrame.numDirLights; ++i) {
		DirectionalLight dirLight = uboPerFrame.directionalLights[i];

		vec3 L = -dirLight.direction;
		vec3 H = normalize(V + L);
		vec3 radiance = dirLight.intensity;

		float NDF = distributionGGX(N, H, roughness);
		float G = geometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		vec3 num = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = num / max(denom, 0.001);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	// Point lights
	for (int i = 0; i < uboPerFrame.numPointLights; ++i) {
		PointLight pointLight = uboPerFrame.pointLights[i];

		vec3 L = normalize(pointLight.position - vs_coords);
		vec3 H = normalize(V + L);

		float distance = length(pointLight.position - vs_coords);
		vec3 radiance = pointLight.intensity * pointLightAttenuation(pointLight, distance);

		float NDF = distributionGGX(N, H, roughness);
		float G = geometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		vec3 num = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = num / max(denom, 0.001);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	// Final color
	vec3 ambient = vec3(0.03) * albedo;
	vec3 finalColor = ambient + Lo;

	output0 = vec4(finalColor, opacity);
}
