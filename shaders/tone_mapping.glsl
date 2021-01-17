#version 430 core

layout (binding = 0) uniform sampler2D hdr_image;
layout (binding = 1) uniform sampler2D hdr_bloom;
layout (binding = 2) uniform sampler2D god_ray;
layout (binding = 3) uniform sampler2D volumetricCloud;

layout (std140, binding = 1) uniform UBO_ToneMapping {
	float exposure;    // cvar: r.tonemapping.exposure
	float gamma;       // cvar: r.gamma
} ubo;

in VS_OUT {
	vec2 screenUV;
} fs_in;

out vec4 color;

void main() {
	ivec2 texelXY = ivec2(gl_FragCoord.xy);

	vec4 c = texelFetch(hdr_image, texelXY, 0);
	c.xyz += textureLod(hdr_bloom, fs_in.screenUV, 0).xyz;
	c.xyz += textureLod(god_ray, fs_in.screenUV, 0).xyz;

	vec4 cloud = texture(volumetricCloud, fs_in.screenUV);
	c.xyz = mix(c.xyz, cloud.xyz, 1.0 - cloud.a);

	// tone mapping
	c.rgb = vec3(1.0) - exp(-c.rgb * ubo.exposure);

	// gamma correction
	c.rgb = pow(c.rgb, vec3(1.0 / ubo.gamma));

	// w component is for depth-of-field. continue to depth_of_field.glsl
	color = c;
}
