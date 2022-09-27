// Unlit text.

// #todo-material: In Ryzen 6800U, interpolants.texcoord is always zero for unlit.
// Maybe look into the assembly code later.
#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT
//#define SHADINGMODEL MATERIAL_SHADINGMODEL_UNLIT

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
	float alpha = texture(fontCache, interpolants.texcoord).r;
	if (alpha < 0.5) {
		discard;
	}
#endif

#if SHADINGMODEL == MATERIAL_SHADINGMODEL_UNLIT
	MaterialAttributes_Unlit attr;

	attr.color = uboMaterial.color;

#elif SHADINGMODEL == MATERIAL_SHADINGMODEL_DEFAULTLIT
	MaterialAttributes_DefaultLit attr;

	attr.albedo    = uboMaterial.color;
	attr.normal    = vec3(0.0, 0.0, 1.0);
	attr.metallic  = 0.0f;
	attr.roughness = 1.0f;
	attr.emissive  = vec3(0.0);
	attr.localAO   = 1.0;
#endif

	return attr;
}
ATTR_END
