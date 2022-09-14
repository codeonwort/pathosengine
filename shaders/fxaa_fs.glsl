#version 460 core

// Just to prevent syntax error highlighting
#ifndef FXAA_PC
	#define FXAA_PC              1
	#define FXAA_GLSL_130        1
	#define FXAA_GREEN_AS_LUMA   1
	#define FXAA_QUALITY__PRESET 23
#endif

#include "nvidia_fxaa.glsl"
#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_FXAA {
	FxaaFloat4 fxaaConsoleRcpFrameOpt;
	FxaaFloat4 fxaaConsoleRcpFrameOpt2;
	FxaaFloat4 fxaaConsole360RcpFrameOpt2;
	FxaaFloat4 fxaaConsole360ConstDir;
	FxaaFloat2 fxaaQualityRcpFrame;
	FxaaFloat fxaaQualitySubpix;
	FxaaFloat fxaaQualityEdgeThreshold;
	FxaaFloat fxaaQualityEdgeThresholdMin;
	FxaaFloat fxaaConsoleEdgeSharpness;
	FxaaFloat fxaaConsoleEdgeThreshold;
	FxaaFloat fxaaConsoleEdgeThresholdMin;
} ubo;

layout (binding = 0) uniform sampler2D sceneColor;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

void main() {
	vec2 screenUV = fs_in.screenUV;
	vec2 inv_size = uboPerFrame.screenResolution.zw;

	vec2 pos = screenUV + 0.5 * inv_size;
	vec4 consolePosPos = vec4(screenUV.x, screenUV.y, screenUV.x + inv_size.x, screenUV.y + inv_size.y);

	outSceneColor = FxaaPixelShader(
		pos, consolePosPos,
		sceneColor, sceneColor, sceneColor,
		ubo.fxaaQualityRcpFrame,
		ubo.fxaaConsoleRcpFrameOpt,
		ubo.fxaaConsoleRcpFrameOpt2,
		ubo.fxaaConsole360RcpFrameOpt2,
		ubo.fxaaQualitySubpix,
		ubo.fxaaQualityEdgeThreshold,
		ubo.fxaaQualityEdgeThresholdMin,
		ubo.fxaaConsoleEdgeSharpness,
		ubo.fxaaConsoleEdgeThreshold,
		ubo.fxaaConsoleEdgeThresholdMin,
		ubo.fxaaConsole360ConstDir);
}
