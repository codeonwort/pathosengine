#version 460 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 6) in;

layout (location = 1) uniform uint cubemapSize;

layout (binding = 0, rgba16f) uniform readonly imageCube inputTexture;
layout (binding = 1, rgba16f) uniform writeonly imageCube outputTexture;

void main() {
	if (gl_GlobalInvocationID.x < cubemapSize && gl_GlobalInvocationID.y < cubemapSize) {
		ivec3 coord = ivec3(gl_GlobalInvocationID);
		vec4 color = imageLoad(inputTexture, coord);
		imageStore(outputTexture, coord, color);
	}
}
