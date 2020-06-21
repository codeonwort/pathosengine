//#version 450 core // uncomment to check grammar

layout (location = 0) out uvec4 packOutput0; // (albedo, normal)
layout (location = 1) out vec4 packOutput1; // (world_position, old_specular)
layout (location = 2) out vec4 packOutput2; // (metallic, roughness, ao, ?)

// VS = view space
void packGBuffer(vec3 albedo, vec3 normalVS, uint materialID, vec3 positionVS, float metallic, float roughness, float localAO) {
	uvec4 out0;
	vec4 out1;
	vec4 out2;

	out0.x = packHalf2x16(albedo.xy);
	out0.y = packHalf2x16(vec2(albedo.z, normalVS.x));
	out0.z = packHalf2x16(normalVS.yz);
	out0.w = materialID;

	out1.xyz = positionVS;
	out1.w = 128.0;

	out2.x = metallic;
	out2.y = roughness;
	out2.z = localAO;

	packOutput0 = out0;
	packOutput1 = out1;
	packOutput2 = out2;
}
