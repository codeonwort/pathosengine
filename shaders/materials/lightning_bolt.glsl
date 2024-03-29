// Lightning bolt effect for world_rc1.

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT
#define NONTRIVIALDEPTH

PARAMETER_CONSTANT(vec3, emissive)
PARAMETER_CONSTANT(float, billboardWidth)
PARAMETER_CONSTANT(float, warpAngle)

PARAMETER_TEXTURE(0, sampler2D, maskTexture)
PARAMETER_TEXTURE(1, sampler2D, warpTexture)

VPO_BEGIN
vec3 getVertexPositionOffset(VertexShaderInput vsi) {
	mat3 viewInv  = transpose(uboPerFrame.viewTransform3x3);

	// #todo: Temp billboard (bad quality; inconsistent thickness)
	vec3 delta = vec3(0.0);
	vec3 right = viewInv * vec3(1.0, 0.0, 0.0);
	float amp = uboMaterial.billboardWidth;
	delta = right * amp * (vsi.texcoord.x - 0.5);

	return delta;
}
VPO_END

ATTR_BEGIN
MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;

	vec2 uv = interpolants.texcoord * vec2(1, 3);

	float time = getWorldTime() * 4;

	vec2 warpDelta = vec2(0.0, time * 0.2);
	float warp = textureLod(warpTexture, uv.yx + warpDelta, 0).r;

	float angle = uboMaterial.warpAngle * PI / 180.0;
	vec2 maskDelta = warp * vec2(cos(angle), sin(angle)) / 2.0;

	float mask = textureLod(maskTexture, uv.yx + maskDelta, 0).r;
#if FRAGMENT_SHADER
	if (mask < 0.5) {
		discard;
	}
#endif

	attr.albedo    = vec3(0.0, 0.0, 0.0);
	attr.normal    = vec3(0.0, 0.0, 1.0);
	attr.metallic  = 0.0f;
	attr.roughness = 0.0f;
	attr.emissive  = uboMaterial.emissive * mask;
	attr.localAO   = 1.0;

	return attr;
}
ATTR_END
