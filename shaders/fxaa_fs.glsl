#version 430 core

#include "nvidia_fxaa.glsl"

layout (binding = 0) uniform sampler2D sceneColor;

in vec2 uv;

uniform vec2 uniform_inv_size;

uniform FxaaFloat2 fxaaQualityRcpFrame;
uniform FxaaFloat4 fxaaConsoleRcpFrameOpt;
uniform FxaaFloat4 fxaaConsoleRcpFrameOpt2;
uniform FxaaFloat4 fxaaConsole360RcpFrameOpt2;
uniform FxaaFloat fxaaQualitySubpix;
uniform FxaaFloat fxaaQualityEdgeThreshold;
uniform FxaaFloat fxaaQualityEdgeThresholdMin;
uniform FxaaFloat fxaaConsoleEdgeSharpness;
uniform FxaaFloat fxaaConsoleEdgeThreshold;
uniform FxaaFloat fxaaConsoleEdgeThresholdMin;
uniform FxaaFloat4 fxaaConsole360ConstDir;

out vec4 out_color;

void main() {
	vec2 pos = uv + 0.5 * uniform_inv_size;
	vec4 consolePosPos = vec4(uv.x, uv.y, uv.x + uniform_inv_size.x, uv.y + uniform_inv_size.y);

	out_color = FxaaPixelShader(
		pos, consolePosPos,
		sceneColor, sceneColor, sceneColor,
		fxaaQualityRcpFrame,
		fxaaConsoleRcpFrameOpt,
		fxaaConsoleRcpFrameOpt2,
		fxaaConsole360RcpFrameOpt2,
		fxaaQualitySubpix,
		fxaaQualityEdgeThreshold,
		fxaaQualityEdgeThresholdMin,
		fxaaConsoleEdgeSharpness,
		fxaaConsoleEdgeThreshold,
		fxaaConsoleEdgeThresholdMin,
		fxaaConsole360ConstDir);
}
