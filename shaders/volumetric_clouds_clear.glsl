#version 450 core

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba16f) writeonly uniform image2D renderTarget;

void main() {
    ivec2 sceneSize = imageSize(renderTarget);
    if (gl_GlobalInvocationID.x >= sceneSize.x || gl_GlobalInvocationID.y >= sceneSize.y) {
		return;
	}

    ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);
    vec4 clearValues = vec4(0.0, 0.0, 0.0, 1.0);

    imageStore(renderTarget, currentTexel, clearValues);
}
