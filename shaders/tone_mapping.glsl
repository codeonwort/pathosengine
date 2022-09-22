#version 460 core

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

out vec4 outSceneColor;

void main() {
	ivec2 texelXY = ivec2(gl_FragCoord.xy);
	vec2 screenUV = fs_in.screenUV;

	vec3 sceneColor = texelFetch(hdr_image, texelXY, 0).rgb;
	vec3 sceneBloom = textureLod(hdr_bloom, screenUV, 0).rgb;
	vec3 godRay     = textureLod(god_ray, screenUV, 0).rgb;

	sceneColor = mix(sceneColor, sceneBloom, 0.04);

	vec4 c = vec4(sceneColor, 0.0);
	c.rgb += godRay;

	vec4 cloud = texture(volumetricCloud, screenUV);
	c.rgb = mix(c.rgb, cloud.rgb, 1.0 - cloud.a);

	// Reinhard tone mapper
	c.rgb = vec3(1.0) - exp(-c.rgb * ubo.exposure);

	// gamma correction
	c.rgb = pow(c.rgb, vec3(1.0 / ubo.gamma));

	outSceneColor = c;
}
