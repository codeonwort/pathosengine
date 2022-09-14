//? #version 460 core

// x = albedo.x, albedo.y
// y = albedo.z, normal.x
// z = normal.y, normal.z
// w = materialID
layout (location = 0) out uvec4 packOutput0;

// x = position.x
// y = position.y
// z = position.z
// w = oldSpecular (128.0)
layout (location = 1) out vec4 packOutput1;

// x = metallic, roughness
// y = localAO, emissive.x
// z = emissive.y, emissive.z
// w = <undefined>
layout (location = 2) out uvec4 packOutput2;

// VS = view space
void packGBuffer(
	vec3 albedo, vec3 normalVS, uint materialID,
	vec3 positionVS, float metallic, float roughness,
	float localAO, vec3 emissive)
{
	uvec4 out0;
	vec4 out1;
	uvec4 out2;

	out0.x = packHalf2x16(albedo.xy);
	out0.y = packHalf2x16(vec2(albedo.z, normalVS.x));
	out0.z = packHalf2x16(normalVS.yz);
	out0.w = materialID;

	out1.xyz = positionVS;
	out1.w = 128.0;

	out2.x = packHalf2x16(vec2(metallic, roughness));
	out2.y = packHalf2x16(vec2(localAO, emissive.x));
	out2.z = packHalf2x16(emissive.yz);

	packOutput0 = out0;
	packOutput1 = out1;
	packOutput2 = out2;
}

bool shouldDiscard(float opacity) {
	return opacity < 0.05;
}
