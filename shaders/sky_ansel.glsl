#version 450 core

layout (location = 0) uniform mat4 viewProj;
layout (location = 1) uniform vec4 screenSize;

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

out VS_OUT {
	vec3 r;
} vs_out;

void main() {
	vs_out.r = position;
	gl_Position = (viewProj * vec4(position, 1)).xyww;
}

#endif

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

layout (binding = 0) uniform sampler2D texSky;

in VS_OUT {
	vec3 r;
} fs_in;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright;

void main() {
	const float PI = 3.14159265359;
	vec3 r0 = normalize(fs_in.r);
	vec3 r = vec3(r0.x, r0.z, -r0.y);
	vec2 tc;
	tc.x = (atan(r.y, r.x) + PI) / PI * 0.5;
	tc.y = acos(r.z) / PI;

	vec3 sky = texture(texSky, tc).xyz;
	out_color = vec4(sky, 1.0);

	out_bright = vec4(0.0);
}

#endif
