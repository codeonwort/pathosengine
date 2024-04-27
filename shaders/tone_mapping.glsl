#version 460 core

//?#define TONE_MAPPER 0
#define TONE_MAPPER_REINHARD 0
#define TONE_MAPPER_ACES     1

layout (binding = 0) uniform sampler2D hdr_image;
layout (binding = 1) uniform sampler2D hdr_bloom;
layout (binding = 2) uniform sampler2D sceneLuminance;

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_ToneMapping {
	float exposureOverride;      // cvar: r.tonemapping.exposureOverride
	float gamma;                 // cvar: r.tonemapping.gamma
	int   useAutoExposure;
	int   sceneLuminanceLastMip;
} ubo;

// --------------------------------------------------------
// Output

out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
const mat3 ACESInputMat = mat3(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mat3(
	 1.60475, -0.53108, -0.07367,
	-0.10208,  1.10813, -0.00605,
	-0.00327, -0.07276,  1.07602);

vec3 RRTAndODTFit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

vec3 ACESFitted(vec3 color)
{
	color = transpose(ACESInputMat) * color;

	// Apply RRT and ODT
	color = RRTAndODTFit(color);

	color = transpose(ACESOutputMat) * color;

	// Clamp to [0, 1]
	color = clamp(color, vec3(0.0), vec3(1.0));

	return color;
}

void main() {
	ivec2 texelXY = ivec2(gl_FragCoord.xy);
	vec2 screenUV = fs_in.screenUV;

	vec3 sceneColor = texelFetch(hdr_image, texelXY, 0).rgb;
	vec3 sceneBloom = textureLod(hdr_bloom, screenUV, 0).rgb;

	// Apply exposure.
	float exposure = 0.0;
	if (ubo.useAutoExposure != 0) {
		float avgLuminance = texelFetch(sceneLuminance, ivec2(0, 0), ubo.sceneLuminanceLastMip).r;
		exposure = 1.0 / (9.6 * avgLuminance);
	} else {
		exposure = ubo.exposureOverride;
	}
	sceneColor *= pow(2.0, exposure);

	// Mix with bloom.
	sceneColor = mix(sceneColor, sceneBloom, 0.04);

	// Apply tonemapping operator.
#if TONE_MAPPER == TONE_MAPPER_REINHARD
	sceneColor = sceneColor / (vec3(1.0) + sceneColor);
	//c.rgb = vec3(1.0) - exp(-c.rgb * manualExposure);
#elif TONE_MAPPER == TONE_MAPPER_ACES
	sceneColor = ACESFitted(sceneColor);
#endif

	// Gamma correction.
	sceneColor = pow(sceneColor, vec3(1.0 / ubo.gamma));

	// Final output.
	outSceneColor = vec4(sceneColor, 0.0);
}
