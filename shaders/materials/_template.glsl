#version 460 core

// --------------------------------------------------------
// Common

#include "common.glsl"
#include "deferred_common.glsl"

#define UBO_BINDING_OBJECT   1
#define UBO_BINDING_MATERIAL 2

// #todo-driver-bug: What's this :/
#define WORKAROUND_RYZEN_6800U_BUG 1

// VERTEX_SHADER or FRAGMENT_SHADER
$NEED SHADERSTAGE

// Should be one of MATERIAL_SHADINGMODEL_XXX in common.glsl.
$NEED SHADINGMODEL

// #todo: Remove unnecessary members
layout (std140, binding = UBO_BINDING_OBJECT) uniform UBO_PerObject {
	// 64 bytes (4 * 16)
	mat4 modelTransform;
	// 64 bytes (4 * 16)
	mat4 mvTransform;
	// 48 bytes (3 * 16)
	mat3 mvTransform3x3;
} uboPerObject;

// The assembler will generate a UBO from PARAMETER_CONSTANT definitions.
$NEED UBO_Material

// Texture parameters
$NEED TEXTURE_PARAMETERS

// Macros that concrete materials can use.
//
// PARAMETER_CONSTANT(type, name)
// - Put into material UBO.
// - ex) PARAMETER_CONSTANT(float, worldTime)
//       PARAMETER_CONSTANT(vec3, playerPos)
//       -> layout (std140, binding = 2) uniform UBO_Material {
//              vec3 playerPos; float worldTime;
//          } ubo;
//
// PARAMETER_TEXTURE(binding, samplerType, name)
// - Declared as a sampler.
// - ex) PARAMETER_TEXTURE(0, sampler2D, tex_albedo)
//       -> layout (binding = 0) uniform sampler2D tex_albedo;

struct MaterialAttributes_Unlit {
	vec3 color;
};

struct MaterialAttributes_DefaultLit {
	vec3 albedo;
	vec3 normal;
	float metallic;
	float roughness;
	vec3 emissive;
	float localAO;
};

struct MaterialAttributes_Translucent {
	vec3 albedo;
	vec3 normal;
	float metallic;
	float roughness;
	vec3 emissive;
	float localAO;
};

#if SHADINGMODEL == MATERIAL_SHADINGMODEL_UNLIT
	#define MaterialAttributes MaterialAttributes_Unlit
#elif SHADINGMODEL == MATERIAL_SHADINGMODEL_DEFAULTLIT
	#define MaterialAttributes MaterialAttributes_DefaultLit
#elif SHADINGMODEL == MATERIAL_SHADINGMODEL_TRANSLUCENT
	#define MaterialAttributes MaterialAttributes_Translucent
#else
	#error "Invalid SHADINGMODEL. See common.glsl"
#endif

// #todo: GLSL spec says I should not use 'location' for inout interface block,
//        but glslangvalidator says I should use one???
// Interpolants (VS to PS)
#if VERTEX_SHADER
	#define INTERPOLANTS_QUALIFIER out
#elif FRAGMENT_SHADER
	#define INTERPOLANTS_QUALIFIER in
#endif
layout (location = 0) INTERPOLANTS_QUALIFIER Interpolants {
	// #todo-material-assembler: Optimize memory bandwidth
	vec3 positionVS;   // view space
	vec3 normal;       // local space
	vec3 tangent;      // local space
	vec3 bitangent;    // local space
	vec2 texcoord;     // local space
} interpolants;

// Controls world position offset.
$NEED getVertexPositionOffset

// Most important output of material shaders.
$NEED getMaterialAttributes

// --------------------------------------------------------
// Vertex shader

#if VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

void main() {
	mat4 model = uboPerObject.modelTransform;
	mat4 view = uboPerFrame.viewTransform;
	mat4 proj = uboPerFrame.projTransform;

	vec4 positionWS = model * vec4(position, 1.0);
	positionWS.xyz += getVertexPositionOffset();

	vec4 positionVS = view * positionWS;

	interpolants.positionVS  = positionVS.xyz;
	interpolants.normal      = normal;
	interpolants.tangent     = tangent;
	interpolants.bitangent   = bitangent;
	interpolants.texcoord    = texcoord;

	gl_Position = proj * positionVS;
}

#endif // VERTEX_SHADER

// --------------------------------------------------------
// Fragment shader

#if FRAGMENT_SHADER

#include "deferred_common_fs.glsl"

vec3 applyNormalMap(vec3 n, vec3 t, vec3 b, vec3 normalmap) {
	vec3 T = normalize(uboPerObject.mvTransform3x3 * t);
	vec3 B = normalize(uboPerObject.mvTransform3x3 * b);
	vec3 N = normalize(uboPerObject.mvTransform3x3 * n);
	mat3 TBN = mat3(T, B, N);
	return TBN * normalize(normalmap);
}

void main() {
	MaterialAttributes attr = getMaterialAttributes();

#if SHADINGMODEL != MATERIAL_SHADINGMODEL_UNLIT
	vec3 detailNormal = applyNormalMap(
		normalize(interpolants.normal),
		normalize(interpolants.tangent),
		normalize(interpolants.bitangent),
		attr.normal);
#endif

#if SHADINGMODEL == MATERIAL_SHADINGMODEL_UNLIT
	packGBuffer(
		vec3(0.0),
		vec3(0.0),
		SHADINGMODEL,
		interpolants.positionVS,
		0.0,
		0.0,
		1.0,
		attr.color);
#endif

#if SHADINGMODEL == MATERIAL_SHADINGMODEL_DEFAULTLIT
	packGBuffer(
		attr.albedo,
		detailNormal,
		SHADINGMODEL,
		interpolants.positionVS,
		attr.metallic,
		attr.roughness,
		attr.localAO,
		attr.emissive);
#endif

#if SHADINGMODEL == MATERIAL_SHADINGMODEL_TRANSLUCENT
	packGBuffer(
		attr.albedo,
		detailNormal,
		SHADINGMODEL,
		interpolants.positionVS,
		attr.metallic,
		attr.roughness,
		attr.localAO,
		attr.emissive);
#endif

#if WORKAROUND_RYZEN_6800U_BUG && SHADINGMODEL == MATERIAL_SHADINGMODEL_DEFAULTLIT
	// #todo-driver-bug: Somehow the line 'out2.z = packHalf2x16(emissive.yz)'
	// in packGBuffer() is bugged only on Ryzen 6800U.
	if (attr.emissive.yz == vec2(0.0)) {
		packOutput2.z = 0;
	}
#endif
}

#endif // FRAGMENT_SHADER
