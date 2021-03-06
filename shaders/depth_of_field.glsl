#version 430 core

layout (location = 0) out vec4 out_color;

// Prefix sum image generated by prefix_sum.glsl
layout (binding = 0) uniform sampler2D input_image;

layout (std140, binding = 1) uniform UBO_DoF {
	float focal_distance;
	float focal_depth;
	float max_radius;
} ubo;

float getFocalDistance() { return ubo.focal_distance; }
float getFocalDepth()    { return ubo.focal_depth; }
float getMaxRadius()     { return ubo.max_radius; }

const ivec2 zero = ivec2(0, 0);

void main() {
	vec2 C = gl_FragCoord.xy;

	vec4 v = texelFetch(input_image, ivec2(gl_FragCoord.xy), 0);

	float m; // radius of filter kernel

	if (v.w == 0.0) {
		m = 0.5;
	} else {
		m = abs(v.w - getFocalDistance());
		m = 0.5 + smoothstep(0.0, getFocalDepth(), m) * getMaxRadius();
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
