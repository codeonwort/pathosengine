#version 460 core

// Calculate prefix sum table for depth of field.

#ifndef BUCKET_SIZE
	// Minimum work group size that OpenGL specification guarantees.
	#define BUCKET_SIZE 1024
#endif

layout (local_size_x = BUCKET_SIZE) in;

shared vec3 shared_data[BUCKET_SIZE * 2];

layout (binding = 0, rgba32f) readonly uniform image2D input_image;
// Remove writeonly as we need to access image region processed by previous bucket.
layout (binding = 1, rgba32f) /*writeonly*/ uniform image2D output_image;

layout (std140, binding = 1) uniform UBO_PrefixSum {
	int fetchOffset;
	int maxImageLength;
} ubo;

void main() {
	uint id = gl_LocalInvocationID.x;
	uint rd_id;
	uint wr_id;
	uint mask;

	if (id > ubo.maxImageLength) {
		return;
	}

	ivec2 P0 = ivec2(id * 2, gl_WorkGroupID.x);
	ivec2 P1 = ivec2(id * 2 + 1, gl_WorkGroupID.x);
	ivec2 fetchOffset = ivec2(ubo.fetchOffset, 0);

	const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
	uint step = 0;

	vec3 i0 = imageLoad(input_image, P0 + fetchOffset).rgb;
	vec3 i1 = imageLoad(input_image, P1 + fetchOffset).rgb;

	shared_data[P0.x] = i0;
	shared_data[P1.x] = i1;

	groupMemoryBarrier();
	barrier();

	for (step = 0; step < steps ; step++) {
		mask = (1 << step) - 1;
		rd_id = ((id >> step) << (step + 1)) + mask;
		wr_id = rd_id + 1 + (id & mask);

		shared_data[wr_id] += shared_data[rd_id];

		groupMemoryBarrier();
		barrier();
	}

	vec3 prevRunResult = vec3(0.0);
	if (ubo.fetchOffset != 0) {
		prevRunResult = imageLoad(output_image, ivec2(gl_WorkGroupID.x, ubo.fetchOffset - 1)).xyz;
	}

	ivec2 storeOffset = ivec2(0, ubo.fetchOffset);
	imageStore(output_image, storeOffset + P0.yx, vec4(prevRunResult + shared_data[P0.x], 0.0));
	imageStore(output_image, storeOffset + P1.yx, vec4(prevRunResult + shared_data[P1.x], 0.0));
}
