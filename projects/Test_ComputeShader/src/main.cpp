#include "pathos/core_minimal.h"
#include "pathos/shader/shader.h"
using namespace pathos;

#include <stdio.h>

void initComputeShader();

int main(int argc, char** argv) {
	EngineConfig config;
	config.windowWidth  = 800;
	config.windowHeight = 600;
	config.rendererType = ERendererType::Deferred;
	config.title        = "Test: Compute Shader";
	Engine::init(&argc, argv, config);

	initComputeShader();

	gEngine->start();

	return 0;
}

void initComputeShader() {
	// benchmark
	
	GLint workGroupSizeX, workGroupSizeY, workGroupSizeZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSizeX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSizeY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSizeZ);

	GLint maxInvocation;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvocation);

	GLint sharedVariableLimit;
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &sharedVariableLimit);

	//void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	//glBindImageTexture(0, tex_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F)
	//glBindImageTexture(1, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F)

	LOG(LogInfo, "===== Compute Shader Capabilities =====");
	LOG(LogInfo, "max work group size: (%d, %d, %d)", workGroupSizeX, workGroupSizeY, workGroupSizeZ);
	LOG(LogInfo, "max work group invocations: %d", maxInvocation);
	LOG(LogInfo, "max shared memory (kb): %d", sharedVariableLimit);
	LOG(LogInfo, "=======================================");

	/* backup
	string cshader = R"(
#version 430 core
layout (local_size_x = 32, local_size_y = 32) in;
//layout (binding = 0, rgba32f) uniform image2D img_in;
//layout (binding = 1) uniform image2D img_out;
void main() {
	//ivec2 p = ivec2(gl_GlobalInvocationID.xy);
	//vec4 texel = imageLoad(img_in, p);
	//texel = vec4(1.0) - texel;
	//imageStore(img_out, p, texel);
}
)";
	*/

	// test compute shader
	std::string cshader = R"(
#version 430 core

layout (local_size_x = 128) in;

layout (binding = 0) coherent buffer block1 {
	uint input_data[gl_WorkGroupSize.x];
};

layout (binding = 1) coherent buffer block2 {
	uint output_data[gl_WorkGroupSize.x];
};

shared uint shared_data[gl_WorkGroupSize.x * 2];

void main() {
	uint id = gl_LocalInvocationID.x;
	uint rd_id, wr_id, mask;

	const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
	uint step = 0;

	shared_data[id * 2] = input_data[id * 2];
	shared_data[id * 2 + 1] = input_data[id * 2 + 1];

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

	output_data[id * 2] = shared_data[id * 2];
	output_data[id * 2 + 1] = shared_data[id * 2 + 1];
}
)";
	GLuint computeProgram = pathos::createComputeProgram(cshader, "CS_Test");
	CHECK(computeProgram);

	// test data
	std::vector<GLuint> subsum_data(128, 1);

	GLuint buf_in;
	glGenBuffers(1, &buf_in);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf_in);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * subsum_data.size(), &subsum_data[0], GL_DYNAMIC_COPY);

	GLuint buf_out;
	glGenBuffers(1, &buf_out);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf_out);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * subsum_data.size(), NULL, GL_DYNAMIC_COPY);

	// run subsum shader
	glUseProgram(computeProgram);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buf_in);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buf_out);
	glDispatchCompute(128, 1, 1);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	// validation
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf_out);
	GLuint* subsum_result = reinterpret_cast<GLuint*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
	for (int i = 0; i < 128; ++i) {
		//assert(subsum_result[i] == i + 1);
		printf("%d ", subsum_result[i]);
	}
	puts("");
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
