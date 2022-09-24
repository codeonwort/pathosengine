#version 460 core

// --------------------------------------------------------
// Common

#define SHADINGMODEL MATERIAL_SHADINGMODEL_DEFAULTLIT

vec3 getVertexPositionOffset() {
	return vec3(0.0);
}

// --------------------------------------------------------
// Vertex shader

#if VERTEX_SHADER

// Nothing here.

#endif // VERTEX_SHADER

// --------------------------------------------------------
// Fragment shader

#if FRAGMENT_SHADER

PARAMETER_TEXTURE(0, sampler2D, tex_albedo)
PARAMETER_TEXTURE(1, sampler2D, tex_normal)
PARAMETER_TEXTURE(2, sampler2D, tex_metallic)
PARAMETER_TEXTURE(3, sampler2D, tex_roughness)
PARAMETER_TEXTURE(4, sampler2D, tex_localAO)

MaterialAttributes getMaterialAttributes() {
	MaterialAttributes_DefaultLit attr;
	vec2 uv = interpolants.texcoord;

	attr.albedo    = texture(tex_albedo, uv);
	attr.normal    = texture(tex_normal, uv);
	attr.metallic  = texture(tex_metallic, uv);
	attr.roughness = texture(tex_roughness, uv);
	attr.emissive  = vec3(0.0);
	attr.localAo   = texture(tex_localAO, uv);

	return attr;
}

#endif // FRAGMENT_SHADER
