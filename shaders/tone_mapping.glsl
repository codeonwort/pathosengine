#version 430 core

layout (binding = 0) uniform sampler2D hdr_image;
layout (binding = 1) uniform sampler2D hdr_bloom;
layout (binding = 2) uniform sampler2D god_ray;

layout (std140, binding = 0) uniform UBO_ToneMapping {
	float exposure;    // cvar: r.tonemapping.exposure
	float gamma;       // cvar: r.gamma
} ubo;

out vec4 color;

void main() {
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec4 c = texelFetch(hdr_image, uv, 0);
	c.xyz += texelFetch(hdr_bloom, uv, 0).xyz;
	c.xyz += texelFetch(god_ray, uv, 0).xyz;

	// tone mapping
	c.rgb = vec3(1.0) - exp(-c.rgb * ubo.exposure);

	// gamma correction
	c.rgb = pow(c.rgb, vec3(1.0 / ubo.gamma));

	// w component is for depth-of-field. continue to depth_of_field.glsl
	color = c;
}
