#version 460 core

//?#define FP16_CRITERIA 0
#include "fsr1/fsr1_wrapper_common.glsl"

#define A_GPU 1
#define A_GLSL 1
#include "fsr1/ffx_a.h"

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0, rgba16f) readonly uniform image2D inImage;
layout (binding = 1, rgba16f) writeonly uniform image2D outImage;

#ifdef A_HALF
AH4 FsrRcasLoadH(ASW2 p){ return AH4(imageLoad(inImage, ASU2(p))); }
void FsrRcasInputH(inout AH1 r,inout AH1 g,inout AH1 b) {}

AH4 FsrRcasLoadHx2(ASW2 p){ return AH4(imageLoad(inImage, ASU2(p))); }
void FsrRcasInputHx2(inout AH2 r,inout AH2 g,inout AH2 b) {}
#endif

AF4 FsrRcasLoadF(ASU2 p){ return AF4(imageLoad(inImage, ASU2(p))); }
void FsrRcasInputF(inout AF1 r,inout AF1 g,inout AF1 b) {}

#if A_HALF
	#define FSR_RCAS_H  1
	#define COLOR_TYPE  AH3
	#define RCAS_KERNEL FsrRcasH
#else
	#define FSR_RCAS_F  1
	#define COLOR_TYPE  AF3
	#define RCAS_KERNEL FsrRcasF
#endif
#include "fsr1/ffx_fsr1.h"

layout (std140, binding = 1) uniform UBO_FSR1 {
	uvec2 renderViewportSize;
	uvec2 containerTextureSize;
	uvec2 upscaledViewportSize;
	float sharpness; // [0.0, 2.0], 0.0 is sharpest
	float _padding0;
} ubo;

void main() {
	AU4 const0;
	FsrRcasCon(const0, ubo.sharpness);

	AU2 gxy = ARmp8x8(gl_LocalInvocationID.x) + AU2(gl_WorkGroupID.x << 4u, gl_WorkGroupID.y << 4u);

	// Result of FsrRcasH() is stored here.
	COLOR_TYPE gamma2Color = COLOR_TYPE(0, 0, 0);

	RCAS_KERNEL(gamma2Color.r, gamma2Color.g, gamma2Color.b, gxy, const0);
	imageStore(outImage, ivec2(gxy), vec4(vec3(gamma2Color), 0.0));

	gxy.x += 8;
	RCAS_KERNEL(gamma2Color.r, gamma2Color.g, gamma2Color.b, gxy, const0);
	imageStore(outImage, ivec2(gxy), vec4(vec3(gamma2Color), 0.0));

	gxy.y += 8;
	RCAS_KERNEL(gamma2Color.r, gamma2Color.g, gamma2Color.b, gxy, const0);
	imageStore(outImage, ivec2(gxy), vec4(vec3(gamma2Color), 0.0));

	gxy.x -= 8;
	RCAS_KERNEL(gamma2Color.r, gamma2Color.g, gamma2Color.b, gxy, const0);
	imageStore(outImage, ivec2(gxy), vec4(vec3(gamma2Color), 0.0));
}
