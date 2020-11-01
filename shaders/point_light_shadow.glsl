#version 450 core

layout (std140, binding = 1) uniform UBO_PointLightShadow {
	mat4 model;
	mat4 viewproj;
	vec4 lightPositionAndZFar;
} ubo;

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

out VS_OUT {
	vec3 wPos;
} vs_out;

void main() {
	vec4 wPos = ubo.model * vec4(position, 1.0);
	vs_out.wPos = wPos.xyz;

	gl_Position = ubo.viewproj * wPos;
}

#endif

////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

in VS_OUT {
	vec3 wPos;
} fs_in;

out vec4 outLinearDepth;

void main() {
	float dist = length(fs_in.wPos - ubo.lightPositionAndZFar.xyz);
	dist = dist / ubo.lightPositionAndZFar.w;

	outLinearDepth = vec4(dist);
}

#endif
