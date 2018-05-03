#version 430 core

layout (binding = 0) uniform sampler2D input_image;

layout (location = 0) out vec4 out_color;

uniform float focal_distance = 50.0;
uniform float focal_depth = 30.0;

void main() {
	vec2 s = 1.0 / textureSize(input_image, 0);
	vec2 C = gl_FragCoord.xy;

	vec4 v = texelFetch(input_image, ivec2(gl_FragCoord.xy), 0);

	float m; // radius of filter kernel

	if(v.w == 0.0){
		m = 0.5;
	}else{
		m = abs(v.w - focal_distance);
		m = 0.5 + smoothstep(0.0, focal_depth, m) * 7.5;
	}
	
	vec2 P0 = vec2(C * 1.0) + vec2(-m, -m);
	vec2 P1 = vec2(C * 1.0) + vec2(-m, m);
	vec2 P2 = vec2(C * 1.0) + vec2(m, -m);
	vec2 P3 = vec2(C * 1.0) + vec2(m, m);

	P0 *= s; P1 *= s; P2 *= s; P3 *= s;

	vec3 a = textureLod(input_image, P0, 0).rgb;
	vec3 b = textureLod(input_image, P1, 0).rgb;
	vec3 c = textureLod(input_image, P2, 0).rgb;
	vec3 d = textureLod(input_image, P3, 0).rgb;

	vec3 f = a - b - c + d;
	m *= 2;
	f /= float(m * m);

	out_color = vec4(f, 1.0);
}