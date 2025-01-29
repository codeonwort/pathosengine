// Dummy material for drawcall merging via indirect draw.
// See depth_prepass.cpp.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_UNLIT
#define USE_INDIRECT_DRAW
#define TRANSFER_DRAW_ID

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_Unlit attr;

	attr.color = vec3(0.9, 0.9, 0.9);

	return attr;
}
ATTR_END
