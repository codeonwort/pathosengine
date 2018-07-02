#version 430 core

// Subsum image generated by subsum.glsl
layout (binding = 0) uniform sampler2D input_image;

layout (location = 0) out vec4 out_color;

uniform float focal_distance = 50.0;
uniform float focal_depth = 300.0;

const ivec2 zero = ivec2(0, 0);

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

	ivec2 P0 = ivec2(C + vec2(-m, -m));
	ivec2 P1 = ivec2(C + vec2(-m, m));
	ivec2 P2 = ivec2(C + vec2(m, -m));
	ivec2 P3 = ivec2(C + vec2(m, m));

	ivec2 extent = textureSize(input_image, 0) - ivec2(1, 1);
	P0 = clamp(P0, zero, extent);
	P1 = clamp(P1, zero, extent);
	P2 = clamp(P2, zero, extent);
	P3 = clamp(P3, zero, extent);
	float denom = float((P3.x - P1.x) * (P3.y - P2.y));

	vec3 a = texelFetch(input_image, P0, 0).rgb;
	vec3 b = texelFetch(input_image, P1, 0).rgb;
	vec3 c = texelFetch(input_image, P2, 0).rgb;
	vec3 d = texelFetch(input_image, P3, 0).rgb;

	vec3 f = a - b - c + d;
	f /= denom;

	out_color = vec4(f, 1.0);
}