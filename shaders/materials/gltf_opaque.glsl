// GLTF OPAQUE material.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

PARAMETER_CONSTANT(vec3, baseColorFactor)
PARAMETER_CONSTANT(float, metallicFactor)
PARAMETER_CONSTANT(float, roughnessFactor)
PARAMETER_CONSTANT(vec3, emissiveFactor)

PARAMETER_TEXTURE(0, sampler2D, baseColorTexture)
PARAMETER_TEXTURE(1, sampler2D, normalTexture)
PARAMETER_TEXTURE(2, sampler2D, metallicRoughnessTexture)
PARAMETER_TEXTURE(3, sampler2D, occlusionTexture)
PARAMETER_TEXTURE(4, sampler2D, emissiveTexture)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;
	vec2 uv = interpolants.texcoord;

	attr.albedo    = texture(baseColorTexture, uv).rgb        * uboMaterial.baseColorFactor;
	attr.normal    = normalize(texture(normalTexture, uv).rgb * 2.0 - vec3(1.0));
	attr.metallic  = texture(metallicRoughnessTexture, uv).r  * uboMaterial.metallicFactor;
	attr.roughness = texture(metallicRoughnessTexture, uv).g  * uboMaterial.roughnessFactor;
	attr.localAO   = texture(occlusionTexture, uv).r;
	attr.emissive  = texture(emissiveTexture, uv).rgb         * uboMaterial.emissiveFactor;

	return attr;
}
ATTR_END
