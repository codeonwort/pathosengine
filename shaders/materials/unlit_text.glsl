// Unlit text.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_UNLIT

PARAMETER_CONSTANT(vec3, color)

PARAMETER_TEXTURE(0, sampler2D, fontCache)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {

// #todo-material: Hacky injection of discard.
// Maybe need separate features like getOpacity() and masked material?
//#if FRAGMENT_SHADER
//	float alpha = texture(fontCache, interpolants.texcoord).r;
//	if (alpha < 0.5) {
//		//discard;
//	}
//#endif

	MaterialAttributes_Unlit attr;

	attr.color = uboMaterial.color * 0.00001;
	
	// texcoord is always zero???
	#if FRAGMENT_SHADER
	attr.color.xy += interpolants.texcoord;
	#endif

	return attr;
}
ATTR_END
