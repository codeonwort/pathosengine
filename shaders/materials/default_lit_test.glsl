// Simple DefaultLit material that samples source textures for each material attribute.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

// #todo-material-assembler: For parsing test. Will be removed.
PARAMETER_CONSTANT(vec3, albedoOverride)
PARAMETER_CONSTANT(float, roughnessOverride)
//PARAMETER_CONSTANT(ivec2, test2a)
//PARAMETER_CONSTANT(uvec2, test2b)
//PARAMETER_CONSTANT(vec4, test4a)
//PARAMETER_CONSTANT(bvec3, test3a)
//PARAMETER_CONSTANT(vec3, test3b)
//PARAMETER_CONSTANT(ivec3, test3c)
//PARAMETER_CONSTANT(float, worldTime)

//PARAMETER_TEXTURE(0, sampler2D, tex_albedo)
//PARAMETER_TEXTURE(1, sampler2D, tex_normal)
//PARAMETER_TEXTURE(2, sampler2D, tex_metallic)
//PARAMETER_TEXTURE(3, sampler2D, tex_roughness)
//PARAMETER_TEXTURE(4, sampler2D, tex_localAO)

VPO_BEGIN
vec3 getVertexPositionOffset() {
	return vec3(0.0);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;
	vec2 uv = interpolants.texcoord;

	#if 0
	attr.albedo    = texture(tex_albedo, uv).rgb;
	attr.normal    = texture(tex_normal, uv).rgb;
	attr.metallic  = texture(tex_metallic, uv).r;
	attr.roughness = texture(tex_roughness, uv).r;
	attr.emissive  = vec3(0.0);
	attr.localAO   = texture(tex_localAO, uv).r;
	#else
	attr.albedo    = uboMaterial.albedoOverride;
	attr.normal    = vec3(0.0, 0.0, 1.0);
	attr.metallic  = 0.0;
	attr.roughness = uboMaterial.roughnessOverride;
	attr.emissive  = vec3(0.0);
	attr.localAO   = 1.0;
	#endif

	return attr;
}
ATTR_END
