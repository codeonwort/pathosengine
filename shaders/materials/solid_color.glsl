// Simple DefaultLit material that uses constant values for each material attribute.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

PARAMETER_CONSTANT(vec3, albedo)
PARAMETER_CONSTANT(float, metallic)
PARAMETER_CONSTANT(float, roughness)
PARAMETER_CONSTANT(vec3, emissive)

VPO_BEGIN
vec3 getVertexPositionOffset() {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;

	attr.albedo    = uboMaterial.albedo;
	attr.normal    = vec3(0.0, 0.0, 1.0);
	attr.metallic  = uboMaterial.metallic;
	attr.roughness = uboMaterial.roughness;
	attr.emissive  = uboMaterial.emissive;
	attr.localAO   = 1.0;

	return attr;
}
ATTR_END
