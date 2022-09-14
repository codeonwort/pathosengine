#version 460 core

layout (binding = 0) uniform sampler2D inputTexture; // sceneColor

layout (location = 0) out vec4 outputColor; // sceneColorDownsample

in VS_OUT {
	vec2 screenUV;
} fs_in;

// #todo-glsl: Parametrize
const float preExposure = 1.0;
const float preExposureInv = 1.0;
const float exposureScale = 1.0;
const float bloomThreshold = 1.0;

float getLuminance(vec3 v) {
	return dot(v, vec3(0.3, 0.59, 0.11));
}

void main() {
	vec4 sceneColor = texture(inputTexture, fs_in.screenUV);

	sceneColor.rgb = min(vec3(256.0 * 256.0), sceneColor.rgb);

	vec3 linearColor = sceneColor.rgb;

	float totalLuminance = getLuminance(linearColor) * exposureScale;
	float bloomLuminance = totalLuminance - bloomThreshold;
	float bloomAmount = clamp(bloomLuminance * 0.5, 0.0, 1.0);

	outputColor = vec4(bloomAmount * linearColor, 0.0) * preExposure;
}
