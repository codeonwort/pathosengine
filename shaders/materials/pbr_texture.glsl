// Simple DefaultLit material that samples PBR textures for each material attribute.
// Also supports fallback constant parametes for absent textures.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

PARAMETER_CONSTANT(vec3, albedoOverride)
PARAMETER_CONSTANT(vec3, normalOverride)
PARAMETER_CONSTANT(float, metallicOverride)
PARAMETER_CONSTANT(float, roughnessOverride)
PARAMETER_CONSTANT(float, localAOOverride)
PARAMETER_CONSTANT(bool, bOverrideAlbedo)
PARAMETER_CONSTANT(bool, bOverrideNormal)
PARAMETER_CONSTANT(bool, bOverrideMetallic)
PARAMETER_CONSTANT(bool, bOverrideRoughness)
PARAMETER_CONSTANT(bool, bOverrideLocalAO)
PARAMETER_CONSTANT(vec3, emissiveConstant)

PARAMETER_TEXTURE(0, sampler2D, albedo)
PARAMETER_TEXTURE(1, sampler2D, normal)
PARAMETER_TEXTURE(2, sampler2D, metallic)
PARAMETER_TEXTURE(3, sampler2D, roughness)
PARAMETER_TEXTURE(4, sampler2D, localAO)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;
	vec2 uv = interpolants.texcoord;

	if (uboMaterial.bOverrideAlbedo) {
		attr.albedo = uboMaterial.albedoOverride;
	} else {
		attr.albedo = texture(albedo, uv).rgb;
	}

	if (uboMaterial.bOverrideNormal) {
		attr.normal = uboMaterial.normalOverride;
	} else {
		attr.normal = normalize(texture(normal, uv).rgb * 2.0 - vec3(1.0));
	}

	if (uboMaterial.bOverrideMetallic) {
		attr.metallic = uboMaterial.metallicOverride;
	} else {
		attr.metallic = texture(metallic, uv).r;
	}

	if (uboMaterial.bOverrideRoughness) {
		attr.roughness = uboMaterial.roughnessOverride;
	} else {
		attr.roughness = texture(roughness, uv).r;
	}

	attr.emissive = uboMaterial.emissiveConstant;

	return attr;
}
ATTR_END
