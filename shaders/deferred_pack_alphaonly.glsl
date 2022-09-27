#version 460 core

#if VERTEX_SHADER
	#define Interpolants out
#elif FRAGMENT_SHADER
	#define Interpolants in
#endif

Interpolants InterpolantsDesc {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 texcoord;
	flat uint material_id;
} interpolants;

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 modelView;
	mat3 modelView3x3;
	vec4 color;
} uboPerObject;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

#include "deferred_common.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexcoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

void main() {
	interpolants.vs_coords   = (uboPerObject.modelView * vec4(inPosition, 1.0)).xyz;
	interpolants.normal      = uboPerObject.modelView3x3 * inNormal;
	interpolants.tangent     = inTangent;
	interpolants.bitangent   = inBitangent;
	interpolants.texcoord    = inTexcoord;
	interpolants.material_id = MATERIAL_ID_ALPHAONLY;

	gl_Position = uboPerFrame.projTransform * (uboPerObject.modelView * vec4(inPosition, 1.0));
}

#endif // VERTEX_SHADER

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

#include "deferred_common_fs.glsl"

layout (binding = 0) uniform sampler2D tex_fontCache;

void main() {
	vec3 albedo = uboPerObject.color.xyz * 0.0001;
	albedo.xy += interpolants.texcoord;
	vec3 normal = normalize(interpolants.normal);
	float metallic = 0.0;
	vec3 emissive = vec3(0.0);
	float roughness = 0.0;
	float localAO = 1.0;

	float alpha = texture2D(tex_fontCache, interpolants.texcoord).r;

	if (alpha < 0.5) {
		discard;
	}

	packGBuffer(
		albedo,
		normal,
		interpolants.material_id,
		interpolants.vs_coords,
		metallic,
		roughness,
		localAO,
		emissive);
}

#endif // FRAGMENT_SHADER
