#version 460 core

#include "deferred_common.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	mat3 mvTransform3x3;
	vec4 albedo;
	vec4 metallic_roughness;
	vec4 transmittance_opacity;
} uboPerObject;

out VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
    vec3 bitangent;
	vec2 texcoord;
} vs_out;

void main() {
	vs_out.vs_coords   = (uboPerObject.mvTransform * vec4(position, 1.0)).xyz;
	vs_out.normal      = normal;
	vs_out.tangent     = tangent;
	vs_out.bitangent   = bitangent;
	vs_out.texcoord    = uv;

	gl_Position = uboPerObject.mvpTransform * vec4(position, 1.0);
}
