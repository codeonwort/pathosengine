// Guard tower for world_rc1.

// Reference for triplanar mapping:
// https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-1-generating-complex-procedural-terrains-using-gpu

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT
#define OUTPUTWORLDNORMAL

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
	vec3 P = interpolants.position * 4.0;
	vec3 N = normalize(interpolants.normal);
	vec3 T = normalize(interpolants.tangent.xyz);
	vec3 B = normalize(interpolants.bitangent);

	vec3 blendWeights = abs(N);
	blendWeights = (blendWeights - vec3(0.2)) * 7;
	blendWeights = max(blendWeights, vec3(0.0));
	blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);
	vec3 NN = N * N;

	vec3 albedoX = texture(albedo, P.yz).rgb;
	vec3 albedoY = texture(albedo, P.zx).rgb;
	vec3 albedoZ = texture(albedo, P.xy).rgb;
	vec3 finalAlbedo = (albedoX * blendWeights.x)
		+ (albedoY * blendWeights.y)
		+ (albedoZ * blendWeights.z);

	vec3 normalX = normalize(texture(normal, P.yz).rgb * 2.0 - vec3(1.0));
	vec3 normalY = normalize(texture(normal, P.zx).rgb * 2.0 - vec3(1.0));
	vec3 normalZ = normalize(texture(normal, P.xy).rgb * 2.0 - vec3(1.0));
	vec3 bumpX = vec3(0.0,       normalX.x, normalX.y);
	vec3 bumpY = vec3(normalY.y, 0.0,       normalY.x);
	vec3 bumpZ = vec3(normalZ.x, normalZ.y, 0.0);
	vec3 finalNormal = normalize(
		N + bumpX * blendWeights.x + bumpY * blendWeights.y + bumpZ * blendWeights.z);

	attr.albedo = finalAlbedo;
	attr.normal = finalNormal;
	attr.metallic = texture(metallic, uv).r;
	attr.roughness = texture(roughness, uv).r;
	attr.localAO = 1.0;
	attr.emissive = vec3(0.0);

	return attr;
}
ATTR_END
