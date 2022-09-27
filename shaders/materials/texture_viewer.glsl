// Unlit material that displays a source texture.
// NOTE: Wow. "unlit_text" and "unlit_texture_viewer" have a same hash.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_UNLIT

PARAMETER_TEXTURE(0, sampler2D, inputTexture)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_Unlit attr;

	attr.color = texture(inputTexture, interpolants.texcoord).rgb;

	return attr;
}
ATTR_END
