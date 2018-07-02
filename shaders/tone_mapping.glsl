#version 430 core

layout (binding = 0) uniform sampler2D hdr_image;
layout (binding = 1) uniform sampler2D hdr_bloom;
layout (binding = 2) uniform sampler2D god_ray;

uniform float exposure = 3.0; // TODO: set this in application-side

out vec4 color;

void main() {
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec4 c = texelFetch(hdr_image, uv, 0);
	c.xyz += texelFetch(hdr_bloom, uv, 0).xyz;
	c.xyz += texelFetch(god_ray, uv, 0).xyz;

	c.rgb = vec3(1.0) - exp(-c.rgb * exposure);

	//c.rgb = c.rgb / (vec3(1.0) + c.rgb);

	// w component is for depth-of-field. continue to depth_of_field.glsl
	color = c;
}
