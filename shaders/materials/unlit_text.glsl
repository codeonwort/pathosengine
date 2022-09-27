// Unlit text.

//#define SHADINGMODEL MATERIAL_SHADINGMODEL_UNLIT
#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

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
#if FRAGMENT_SHADER
	// #todo-material: interpolants.texcoord is zero for unlit?
	float alpha = texture(fontCache, interpolants.texcoord).r;
	if (alpha < 0.5) {
		discard;
	}
#endif

	//MaterialAttributes_Unlit attr;

	//attr.color = uboMaterial.color * 0.00001;
	//// texcoord is always zero???
	//#if FRAGMENT_SHADER
	//attr.color.xy += interpolants.texcoord;
	//#endif

	MaterialAttributes_DefaultLit attr;
	attr.albedo    = uboMaterial.color;
	attr.normal    = vec3(0.0, 0.0, 1.0);
	attr.metallic  = 0.0f;
	attr.roughness = 1.0f;
	attr.emissive  = vec3(0.0);
	attr.localAO   = 1.0;

	return attr;
}
ATTR_END
