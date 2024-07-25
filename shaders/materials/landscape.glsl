// Landscape material

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT
#define TRANSFERDRAWID

PARAMETER_TEXTURE(0, sampler2D, albedo)
PARAMETER_TEXTURE(1, sampler2D, heightmap)

EMBED_GLSL_BEGIN
struct SectorParameter {
	vec4  uvBounds;
	float offsetX;
	float offsetY;
	float _pad0;
	float _pad1;
};

layout (std140, binding = 0) readonly buffer SSBO_SectorParameter {
	SectorParameter sectorParameters[];
};

vec2 getNormalizedUV(vec2 localUV, vec4 uvBounds) {
	// y-axis messed up
	return mix(uvBounds.xw, uvBounds.zy, localUV);
}
EMBED_GLSL_END

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	SectorParameter sector = sectorParameters[gl_DrawID];
	vec2 uv = getNormalizedUV(vsi.texcoord, sector.uvBounds);
	float heightFactor = texture(heightmap, uv).r;

	return vec3(sector.offsetX, 30.0f * heightFactor, sector.offsetY);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	SectorParameter sector = sectorParameters[interpolants.drawID];
	vec2 uv = getNormalizedUV(interpolants.texcoord, sector.uvBounds);

	MaterialAttributes_DefaultLit attr;

	attr.albedo    = texture(albedo, uv).rgb;
	//attr.albedo    = texture(heightmap, uv).rrr;
	// #wip-landscape: Provide normalmap or derive from heightmap
	attr.normal    = vec3(0.0, 0.0, 1.0);
	attr.metallic  = 0.0;
	attr.roughness = 0.9;
	attr.emissive  = vec3(0.0, 0.0, 0.0);
	attr.localAO   = 1.0;

	return attr;
}
ATTR_END
