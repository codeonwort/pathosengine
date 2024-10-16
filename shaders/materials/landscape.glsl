// Landscape material

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT
#define TRANSFER_DRAW_ID

PARAMETER_TEXTURE(0, sampler2D, albedo)
PARAMETER_TEXTURE(1, sampler2D, heightmap)
PARAMETER_CONSTANT(float, heightmapMultiplier)
PARAMETER_CONSTANT(int, sectorCountX)
PARAMETER_CONSTANT(int, sectorCountY)
PARAMETER_CONSTANT(int, baseDivisions)
PARAMETER_CONSTANT(int, debugMode)

EMBED_GLSL_BEGIN
struct SectorParameter {
	vec4  uvBounds;
	float offsetX;
	float offsetY;
	uint lod;
	float _pad0;
};

layout (std140, binding = 0) readonly buffer SSBO_SectorParameter {
	SectorParameter sectorParameters[];
};

vec2 getNormalizedUV(vec2 localUV, vec4 uvBounds) {
	// y-axis messed up
	return mix(uvBounds.xw, uvBounds.zy, localUV);
}

ivec2 getSectorCoords(int glDrawID) {
	return ivec2(glDrawID % uboMaterial.sectorCountX, glDrawID / uboMaterial.sectorCountX);
}
int getSectorLinearIndex(ivec2 sectorCoords) {
	return sectorCoords.y * uboMaterial.sectorCountX + sectorCoords.x;
}

EMBED_GLSL_END

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	SectorParameter sector = sectorParameters[gl_DrawID];
	vec2 uv = getNormalizedUV(vsi.texcoord, sector.uvBounds);
	
	float heightFactor = texture(heightmap, uv).r; // [0.0, 1.0]

	vec4 borders = vec4(vsi.texcoord, vec2(1.0) - vsi.texcoord);
	bvec4 borderFlags = lessThan(abs(borders), vec4(0.001));
	bool currentAtBorder = any(borderFlags);
	ivec2 sectorCoords = getSectorCoords(gl_DrawID);

	ivec2 neighborCoords = sectorCoords;
	bool fixY = false;
	if (borderFlags.x) { neighborCoords.x -= 1; }
	else if (borderFlags.y) { neighborCoords.y += 1; fixY = true; }
	else if (borderFlags.z) { neighborCoords.x += 1; }
	else if (borderFlags.w) { neighborCoords.y -= 1; fixY = true; }
	bool currentAtCorner = any(borderFlags.xz) && any(borderFlags.yw);
	bool neighborExists = all(greaterThanEqual(neighborCoords, ivec2(0, 0)) && lessThan(neighborCoords, ivec2(uboMaterial.sectorCountX, uboMaterial.sectorCountY)));

	// Fix T-junction.
	if (currentAtBorder && !currentAtCorner && neighborExists) {
		int neighborSectorIndex = getSectorLinearIndex(neighborCoords);
		SectorParameter neighbor = sectorParameters[neighborSectorIndex];
		if (sector.lod < neighbor.lod) {
			float divs = float(uboMaterial.baseDivisions >> neighbor.lod);
			vec2 off = vec2(fixY, !fixY) * vec2(1.0 / divs, 1.0 / divs);

			vec2 uv1 = floor(vsi.texcoord * divs) / divs;
			vec2 uv2 = uv1 + off;
			vec2 ratioXY = (uv2 - vsi.texcoord) / off;
			float ratio = fixY ? ratioXY.x : ratioXY.y;

			uv1 = getNormalizedUV(uv1, sector.uvBounds);
			uv2 = getNormalizedUV(uv2, sector.uvBounds);

			float height1 = texture(heightmap, uv1).r;
			float height2 = texture(heightmap, uv2).r;
			heightFactor = mix(height1, height2, 1.0 - ratio);
		}
	}

	heightFactor *= uboMaterial.heightmapMultiplier;

	return vec3(sector.offsetX, heightFactor, sector.offsetY);
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	SectorParameter sector = sectorParameters[interpolants.drawID];
	vec2 uv = getNormalizedUV(interpolants.texcoord, sector.uvBounds);

	MaterialAttributes_DefaultLit attr;

	vec3 lodColors[] = { vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) };
	vec3 baseColor = texture(albedo, uv).rgb;

	if (uboMaterial.debugMode == 1) {
		baseColor = texture(albedo, uv).rgb * lodColors[sector.lod % 3]; // Visualize LOD
	} else if (uboMaterial.debugMode == 2) {
		baseColor = vec3(uv, 0.0);
	} else if (uboMaterial.debugMode == 3) {
		baseColor = vec3(interpolants.texcoord, 0.0);
	}

	attr.albedo    = baseColor;
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
