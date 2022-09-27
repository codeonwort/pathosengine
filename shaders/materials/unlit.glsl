// Simple Unlit material that outputs a constant color.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_UNLIT

PARAMETER_CONSTANT(vec3, color)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_Unlit attr;

	attr.color = uboMaterial.color;

	return attr;
}
ATTR_END
