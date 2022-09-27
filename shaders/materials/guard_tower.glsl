// Guard tower for world_rc1.
// Uses triplanar mapping.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

PARAMETER_TEXTURE(0, sampler2D, albedo)
PARAMETER_TEXTURE(1, sampler2D, normal)
PARAMETER_TEXTURE(2, sampler2D, metallic)
PARAMETER_TEXTURE(3, sampler2D, roughness)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;
	vec2 uv = interpolants.texcoord;

	// #todo: Same process for metallic and roughness
	//        (but they are constants for now, so no matters)
	// #todo: Apply at least model transform
	vec3 P = interpolants.position * 2.0;
	vec3 N = normalize(interpolants.normal);
	vec3 T = normalize(interpolants.tangent);
	vec3 B = normalize(interpolants.bitangent);
	vec3 NN = N * N;
	vec3 albedoX = texture(albedo, P.zy).rgb;
	vec3 albedoY = texture(albedo, P.zx).rgb;
	vec3 albedoZ = texture(albedo, P.xy).rgb;
	vec3 finalAlbedo = (albedoX * NN.x) + (albedoY * NN.y) + (albedoZ * NN.z);

	// #todo: Discontinuous, but original vertex normals are discontinuous...
	//        Need to refine the original 3D model.
	vec3 normalX = normalize(texture(normal, P.zy).rgb * 2.0 - vec3(1.0));
	vec3 normalY = normalize(texture(normal, P.zx).rgb * 2.0 - vec3(1.0));
	vec3 normalZ = normalize(texture(normal, P.xy).rgb * 2.0 - vec3(1.0));
	vec3 finalNormal = normalize((normalX * NN.x) + (normalY * NN.y) + (normalZ * NN.z));
	//vec3 finalNormal = vec3(0, 0, 1);

	attr.albedo = finalAlbedo;
	attr.normal = finalNormal;
	attr.metallic = texture(metallic, uv).r;
	attr.roughness = texture(roughness, uv).r;
	attr.localAO = 1.0;
	attr.emissive = vec3(0.0);

	return attr;
}
ATTR_END
