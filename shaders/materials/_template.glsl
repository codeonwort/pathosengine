#version 460 core

// --------------------------------------------------------
// Common

#include "core/common.glsl"
#include "deferred_common.glsl"

#define UBO_BINDING_OBJECT    1
#define UBO_BINDING_MATERIAL  2
#define UBO_BINDING_LIGHTINFO 3

// #todo-driver-bug: What's this :/
#define WORKAROUND_RYZEN_6800U_BUG 1

// VERTEX_SHADER or FRAGMENT_SHADER
$NEED SHADERSTAGE

// Material shaders should define this.
// Should be one of MATERIAL_SHADINGMODEL_XXX in common.glsl.
$NEED SHADINGMODEL

// Material shaders may define optional symbols:
// [NONTRIVIALDEPTH]
// - Depth write is not trivial (MVP * localPos).
// - Textures and other vertex attributes will be bound.
// [OUTPUTWORLDNORMAL]
// - getMaterialAttributes() returns world normal, not local normal.
// - Normal mapping will be skipped.
// [SKYBOXMATERIAL]
// - This material is not for static meshes, but for skybox.

$NEED OUTPUTWORLDNORMAL
$NEED SKYBOXMATERIAL

#define FORWARD_SHADING (SKYBOXMATERIAL || SHADINGMODEL == MATERIAL_SHADINGMODEL_TRANSLUCENT)

// 128 bytes
layout (std140, binding = UBO_BINDING_OBJECT) uniform UBO_PerObject {
	mat4 modelTransform;
	mat4 prevModelTransform;
} uboPerObject;

// The assembler will generate a UBO from PARAMETER_CONSTANT definitions.
$NEED UBO_Material

#if (FORWARD_SHADING && !SKYBOXMATERIAL)
#define MAX_DIRECTIONAL_LIGHTS     2
#define MAX_POINT_LIGHTS           8
layout (std140, binding = UBO_BINDING_LIGHTINFO) uniform UBO_LightInfo {
	ivec4            numLightSources; // (directional, point, ?, ?)
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight       pointLights[MAX_POINT_LIGHTS];
} uboLight;
#endif

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
	float roughness;
	vec3 transmittance;
};

#if SKYBOXMATERIAL || (SHADINGMODEL == MATERIAL_SHADINGMODEL_UNLIT)
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
// -> Ryzen 6800U is bugged if I specify 'location' here. Comment it out.
// Interpolants (VS to PS)
#if VERTEX_SHADER
	#define INTERPOLANTS_QUALIFIER out
#elif FRAGMENT_SHADER
	#define INTERPOLANTS_QUALIFIER in
#endif
/*layout (location = 0)*/ INTERPOLANTS_QUALIFIER Interpolants {
	// #todo-material-assembler: Optimize memory bandwidth
	vec3 positionVS;   // view space
	vec3 position;     // local space
	vec3 normal;       // local space
	vec3 tangent;      // local space
	vec3 bitangent;    // local space
	vec2 texcoord;     // local space
	vec4 clipPos;      // clip space
	vec4 prevClipPos;  // clip space
} interpolants;

struct VertexShaderInput {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 texcoord;
};

// All inputs in local space of the object
vec3 applyNormalMap(vec3 n, vec3 t, vec3 b, vec3 normalmap) {
	mat3 model = mat3(uboPerObject.modelTransform);
	model = inverse(transpose(model)); // Cannot assure uniform scaling.
	vec3 T = normalize(model * t);
	vec3 B = normalize(model * b);
	vec3 N = normalize(model * n);
	mat3 TBN = mat3(T, B, N);
	return TBN * normalize(normalmap);
}

#if FORWARD_SHADING
#include "core/brdf.glsl"
#endif

// Controls world position offset.
$NEED getVertexPositionOffset

// Most important output of material shaders.
$NEED getMaterialAttributes

// Forward shading only.
$NEED getSceneColor

// --------------------------------------------------------
// Vertex shader

#if VERTEX_SHADER

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexcoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

void main_staticMesh() {
	mat4 model = uboPerObject.modelTransform;
	mat4 view = uboPerFrame.viewTransform;
	mat4 proj_view = uboPerFrame.viewProjTransform;

	vec4 positionWS = model * vec4(inPosition, 1.0);
	vec4 prevPositionWS = uboPerObject.prevModelTransform * vec4(inPosition, 1.0);

	VertexShaderInput vsi;
	vsi.position  = inPosition;
	vsi.texcoord  = inTexcoord;
	vsi.normal    = inNormal;
	vsi.tangent   = inTangent;
	vsi.bitangent = inBitangent;

	positionWS.xyz += getVertexPositionOffset(vsi);

	vec4 positionVS = view * positionWS;

	interpolants.positionVS  = positionVS.xyz;
	interpolants.position    = inPosition;
	interpolants.normal      = inNormal;
	interpolants.tangent     = inTangent;
	interpolants.bitangent   = inBitangent;
	interpolants.texcoord    = inTexcoord;

	// #todo: Precision issue.
	// See SIGGRAPH2012 "Creating Vast Game Worlds" (p.11)
	vec4 positionCS = proj_view * positionWS;
	positionCS.xy += uboPerFrame.temporalJitter.xy * positionCS.w;

	interpolants.clipPos = positionCS;
	interpolants.prevClipPos = uboPerFrame.prevViewProjTransform * prevPositionWS;

	gl_Position = positionCS;
}

void main_skybox() {
	interpolants.normal = inPosition;
	
	gl_Position = uboPerObject.modelTransform * vec4(inPosition, 1.0);

	if (uboPerFrame.bReverseZ == 1) {
		gl_Position.z = 0.0;
	}
}

void main() {
#if SKYBOXMATERIAL
	main_skybox();
#else
	main_staticMesh();
#endif
}

#endif // VERTEX_SHADER

// --------------------------------------------------------
// Fragment shader

#if FRAGMENT_SHADER

#if !FORWARD_SHADING && !SKYBOXMATERIAL
#include "deferred_common_fs.glsl"
#else
// #todo: Wrap with forward_common.glsl?
layout (location = 0) out vec4 outSceneColor;
#endif

void main() {
	MaterialAttributes attr = getMaterialAttributes();

#if SHADINGMODEL != MATERIAL_SHADINGMODEL_UNLIT
	#if OUTPUTWORLDNORMAL
		vec3 worldNormal = attr.normal;
	#else
		vec3 worldNormal = applyNormalMap(
			normalize(interpolants.normal),
			normalize(interpolants.tangent),
			normalize(interpolants.bitangent),
			attr.normal);
	#endif
	vec3 normalVS = (uboPerFrame.viewTransform * vec4(worldNormal, 0.0)).xyz;
#endif

#if !SKYBOXMATERIAL && (SHADINGMODEL == MATERIAL_SHADINGMODEL_UNLIT)
	packGBuffer(
		attr.color, // Unlit color into albedo
		vec3(0.0),
		SHADINGMODEL,
		interpolants.positionVS,
		0.0,
		0.0,
		1.0,
		vec3(0.0));
#endif

#if SHADINGMODEL == MATERIAL_SHADINGMODEL_DEFAULTLIT
	packGBuffer(
		attr.albedo,
		normalVS,
		SHADINGMODEL,
		interpolants.positionVS,
		attr.metallic,
		attr.roughness,
		attr.localAO,
		attr.emissive);
#endif

#if SKYBOXMATERIAL
	outSceneColor = vec4(attr.color, 1.0);
#elif SHADINGMODEL == MATERIAL_SHADINGMODEL_TRANSLUCENT
	outSceneColor = getSceneColor(attr);
#endif

#if WORKAROUND_RYZEN_6800U_BUG && SHADINGMODEL == MATERIAL_SHADINGMODEL_DEFAULTLIT
	// #todo-driver-bug: Somehow the line 'out2.z = packHalf2x16(emissive.yz)'
	// in packGBuffer() is bugged only on Ryzen 6800U.
	if (attr.emissive.yz == vec2(0.0)) {
		packOutput2.z = 0;
	}
#endif

#if WORKAROUND_RYZEN_6800U_BUG && SHADINGMODEL == MATERIAL_SHADINGMODEL_UNLIT && !SKYBOXMATERIAL
	packOutput2 = uvec4(0);
#endif

#if !FORWARD_SHADING && !SKYBOXMATERIAL
	vec2 v1 = (interpolants.clipPos.xy / interpolants.clipPos.w) * 0.5 + vec2(0.5, 0.5);
	vec2 v0 = (interpolants.prevClipPos.xy / interpolants.prevClipPos.w) * 0.5 + vec2(0.5, 0.5);
	outVelocityMap = v1 - v0;
#endif
}

#endif // FRAGMENT_SHADER
