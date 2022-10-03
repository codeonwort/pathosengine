#version 460 core

#define A_GPU 1
#define A_GLSL 1
//#define A_HALF 1
#include "fsr1/ffx_a.h"

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0) uniform sampler2D inImage;
layout (binding = 1, rgba16f) writeonly uniform image2D outImage;

// fp16
#ifdef A_HALF
AH4 FsrEasuRH(AF2 p) { return textureGather(inImage, p, 0); }
AH4 FsrEasuGH(AF2 p) { return textureGather(inImage, p, 1); }
AH4 FsrEasuBH(AF2 p) { return textureGather(inImage, p, 2); }
#endif
// fp32
AF4 FsrEasuRF(AF2 p) { return textureGather(inImage, p, 0); }
AF4 FsrEasuGF(AF2 p) { return textureGather(inImage, p, 1); }
AF4 FsrEasuBF(AF2 p) { return textureGather(inImage, p, 2); }

#define FSR_EASU_F 1
#include "fsr1/ffx_fsr1.h"

layout (std140, binding = 1) uniform UBO_FSR1 {
	uvec2 renderViewportSize;
	uvec2 containerTextureSize;
	uvec2 upscaledViewportSize;
	float sharpness; // [0.0, 2.0], 0.0 is sharpest
	float _padding0;
} ubo;

void main() {
	AU4 const0, const1, const2, const3;
	FsrEasuCon(const0, const1, const2, const3,
		float(ubo.renderViewportSize.x),
		float(ubo.renderViewportSize.y),
		float(ubo.containerTextureSize.x),
		float(ubo.containerTextureSize.y),
		float(ubo.upscaledViewportSize.x),
		float(ubo.upscaledViewportSize.y));

	AU2 gxy = ARmp8x8(gl_LocalInvocationID.x) + AU2(gl_WorkGroupID.x << 4u, gl_WorkGroupID.y << 4u);

	// Result of FsrEasuH() is stored here.
	AF3 gamma2Color = AF3(0, 0, 0);

	FsrEasuF(gamma2Color, gxy, const0, const1, const2, const3);
	imageStore(outImage, ivec2(gxy), vec4(gamma2Color, 0.0));

	// #todo-fsr1: Is writing to an OOB coord a NOP?
	gxy.x += 8;
	FsrEasuF(gamma2Color, gxy, const0, const1, const2, const3);
	imageStore(outImage, ivec2(gxy), vec4(gamma2Color, 0.0));

	gxy.y += 8;
	FsrEasuF(gamma2Color, gxy, const0, const1, const2, const3);
	imageStore(outImage, ivec2(gxy), vec4(gamma2Color, 0.0));

	gxy.x -= 8;
	FsrEasuF(gamma2Color, gxy, const0, const1, const2, const3);
	imageStore(outImage, ivec2(gxy), vec4(gamma2Color, 0.0));
}
