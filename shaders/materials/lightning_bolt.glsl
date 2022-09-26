// Lightning bolt effect for world_rc1.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

PARAMETER_CONSTANT(vec3, emissive)
PARAMETER_CONSTANT(float, billboardWidth)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	// #todo: Temp billboard (bad quality; inconsistent thickness)
	vec3 delta = vec3(0.0);
	vec3 right = inverse(uboPerObject.mvTransform3x3) * vec3(1.0, 0.0, 0.0);
	delta = right * uboMaterial.billboardWidth * (vsi.texcoord.x - 0.5);
	return delta;
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;

	attr.albedo    = vec3(0.0, 0.0, 0.0);
	attr.normal    = vec3(0.0, 0.0, 1.0);
	attr.metallic  = 0.0f;
	attr.roughness = 0.0f;
	attr.emissive  = uboMaterial.emissive;
	attr.localAO   = 1.0;

	return attr;
}
ATTR_END
