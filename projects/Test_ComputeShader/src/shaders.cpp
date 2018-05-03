#include "shaders.h"
#include <string>
#include <cassert>

DepthOfField::DepthOfField() {
	//
}

DepthOfField::~DepthOfField() {
	glDeleteProgram(program_subsum2D);
	glDeleteProgram(program_blur);
}

void DepthOfField::createShaders() {
	// compute program. output is transposed.
	program_subsum2D = createSubsumShader();

	// program with vertex and fragment shaders
	program_blur = createBlurShader();
}

GLuint DepthOfField::createSubsumShader() {
	std::string src = R"(
#version 430 core

layout (local_size_x = 1024) in;

shared float shared_data[gl_WorkGroupSize.x * 2];

layout (binding = 0, r32f) readonly uniform image2D input_image;
layout (binding = 1, r32f) writeonly uniform image2D output_image;

void main() {
	uint id = gl_LocalInvocationID.x;
	uint rd_id, wr_id, mask;
	ivec2 P = ivec2(id * 2, gl_WorkGroupID.x);

	const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
	uint step = 0;

	shared_data[id * 2] = imageLoad(input_image, P).r;
	shared_data[id * 2 + 1] = imageLoad(input_image, P + ivec2(1, 0)).r;

	barrier();
	memoryBarrierShared();

	for(step = 0; step < steps ; step++){
		mask = (1 << step) - 1;
		rd_id = ((id >> step) << (step + 1)) + mask;
		wr_id = rd_id + 1 + (id & mask);

		shared_data[wr_id] += shared_data[rd_id];

		barrier();
		memoryBarrierShared();
	}

	imageStore(output_image, P.yx, vec4(shared_data[id * 2]));
	imageStore(output_image, P.yx + ivec2(0, 1), vec4(shared_data[id * 2 + 1]));
}
)";
	GLuint program = pathos::createComputeProgram(src);
	assert(program);
	return program;
}

GLuint DepthOfField::createBlurShader() {
	std::string vs = R"(
#version 430 core

layout (location = 0) in vec3 position;

void main() {
	gl_Position = vec4(position, 1);
}
)";

	std::string fs = R"(
#version 430 core

layout (binding = 0) uniform sampler2D input_image;

layout (location = 0) out vec4 out_color;

uniform float focal_distance = 50.0;
uniform float focal_depth = 30.0;

void main() {
	vec2 s = 1.0 / textureSize(input_image, 0);
	vec2 C = gl_FragCoord.xy;

	vec4 v = texelFetch(input_image, ivec2(gl_FragCoord.xy), 0).rgba;

	float m; // radius of filter kernel

	if(v.w == 0.0){
		m = 0.5;
	}else{
		m = abs(v.w - focal_distance);
		m = 0.5 + smoothstep(0.0, focal_depth, m) * 7.5;
	}
	vec2 P0 = vec2(C * 1.0) + vec2(-m, -m);
	vec2 P1 = vec2(C * 1.0) + vec2(-m, m);
	vec2 P2 = vec2(C * 1.0) + vec2(m, -m);
	vec2 P3 = vec2(C * 1.0) + vec2(m, m);

	P0 *= s; P1 *= s; P2 *= s; P3 *= s;

	vec3 a = textureLod(input_image, P0, 0).rgb;
	vec3 b = textureLod(input_image, P1, 0).rgb;
	vec3 c = textureLod(input_image, P2, 0).rgb;
	vec3 d = textureLod(input_image, P3, 0).rgb;

	vec3 f = a - b - c + d;
	m *= 2;
	f /= float(m * m);

	out_color = vec4(f, 1.0);
}
)";

	GLuint program = pathos::createProgram(vs, fs);
	assert(program);
	return program;
}
