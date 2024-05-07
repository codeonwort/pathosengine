// Simple Translucent material that represents constant color.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_TRANSLUCENT

PARAMETER_CONSTANT(vec3, albedo)
PARAMETER_CONSTANT(float, roughness)
PARAMETER_CONSTANT(vec3, transmittance)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_Translucent attr;

	attr.albedo        = uboMaterial.albedo;
	attr.normal        = vec3(0.0, 0.0, 1.0);
	attr.roughness     = uboMaterial.roughness;
	attr.transmittance = uboMaterial.transmittance;

	return attr;
}
ATTR_END

FORWARDSHADING_BEGIN
vec4 getSceneColor(MaterialAttributes_Translucent attr) {
	vec3 albedo        = attr.albedo; // vec3(0.1, 0.1, 0.2);
	float metallic     = 0.0;
	float roughness    = attr.roughness;
	vec3 transmittance = attr.transmittance;
	float opacity      = 1.0;

	vec3 vs_coords = interpolants.positionVS;
	vec3 N = normalize(attr.normal);
	vec3 V = normalize(uboPerFrame.cameraPositionVS - vs_coords);

	// Fake glass material (well I don't remember where this formula came from?)
	{
		vec3 L = -uboLight.directionalLights[0].vsDirection;
		vec3 H = normalize(V + L);
		float fakeFresnel = fresnelSchlick(max(dot(H, V), 0.0), vec3(3.0)).x;

		metallic = mix(0.05, 1.0, fakeFresnel);
		roughness = 0.0;
		opacity = mix(0.05, 0.2, fakeFresnel);
		float refraction = mix(1.2, 0.8, fakeFresnel);
	}

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(albedo, vec3(1.0)), metallic);

	vec3 Lo = vec3(0.0);

	// Directional lights
	for (int i = 0; i < uboLight.numLightSources.x; ++i) {
		DirectionalLight dirLight = uboLight.directionalLights[i];

		vec3 L = -dirLight.vsDirection;
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
	for (int i = 0; i < uboLight.numLightSources.y; ++i) {
		PointLight pointLight = uboLight.pointLights[i];

		vec3 L = normalize(pointLight.positionVS - vs_coords);
		vec3 H = normalize(V + L);

		float distance = length(pointLight.positionVS - vs_coords);
		float falloff = pointLightFalloff(pointLight.attenuationRadius, distance);
		vec3 radiance = pointLight.intensity * falloff;

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

	// #todo-translucency: Even generates NaN
	finalColor = max(vec3(0.0), finalColor);

	return vec4(finalColor, opacity);
}
FORWARDSHADING_END