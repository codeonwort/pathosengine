// Landscape material

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

PARAMETER_CONSTANT(vec3, albedo)
PARAMETER_CONSTANT(float, metallic)
PARAMETER_CONSTANT(float, roughness)
PARAMETER_CONSTANT(vec3, emissive)

EMBED_GLSL_BEGIN
struct SectorParameter {
	float offsetX;
	float offsetY;
	float _pad0;
	float _pad1;
};
layout (std140, binding = 0) readonly buffer SSBO_SectorParameter {
	SectorParameter sectorParameters[];
};
EMBED_GLSL_END

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	SectorParameter sector = sectorParameters[gl_DrawID];
	return vec3(sector.offsetX, 0.0, sector.offsetY);
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
