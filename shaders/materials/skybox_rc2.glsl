// Skybox material.

// Skybox should be unlit
#define SHADINGMODEL MATERIAL_SHADINGMODEL_UNLIT
#define SKYBOXMATERIAL

PARAMETER_CONSTANT(float, skyIntensity)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_Unlit attr;

	vec3 viewDir = normalize(interpolants.normal);

	const vec4 cones[8] = {
		vec4(+1.0, 0.5, 0.0, 75.0), // xyz = direction, w = cone half angle (degrees)
		vec4(-0.8, -0.2, 0.1, 85.0),
		vec4(0.3, +1.0, 0.0, 75.0),
		vec4(-0.2, -1.0, 0.3, 75.0),
		vec4(0.0, 0.1, +1.0, 75.0),
		vec4(0.3, -0.2, -1.0, 75.0),
		vec4(1.0, -1.0, -0.5, 75.0),
		vec4(0.1, 0.9, 0.7, 65.0),
	};
	vec3 finalColor = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < 8; ++i) {
		vec3 coneDir = normalize(cones[i].xyz);
		float ang = acos(dot(viewDir, coneDir));
		float maxAngle = cones[i].w * PI / 180.0;
		if (ang <= maxAngle) {
			vec3 coneColor = 0.5 + 0.5 * (-coneDir);
			finalColor += 1.1 * (1.0 - ang / maxAngle) * coneColor;
		}
	}

	//attr.color = 0.5 + 0.5 * viewDir;
	attr.color = uboMaterial.skyIntensity * finalColor;

	return attr;
}
ATTR_END
