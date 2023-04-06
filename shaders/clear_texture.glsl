#version 460 core

#define TEXTURE_DIMENSION_2D   0
#define TEXTURE_DIMENSION_CUBE 1
#if TEXTURE_DIMENSION == TEXTURE_DIMENSION_2D
    #define IMAGE_TYPE image2D
    #define NUM_LAYERS 1
#elif TEXTURE_DIMENSION == TEXTURE_DIMENSION_CUBE
    #define IMAGE_TYPE imageCube
    #define NUM_LAYERS 6
#else
    #error Invalid TEXTURE_DIMENSION.
#endif

#if NUM_COMPONENTS == 4
    #define TEXTURE_FORMAT   rgba16f
    #define CLEAR_VALUE_TYPE vec4
#elif
    #error Invalid NUM_COMPONENTS.
#endif

layout (local_size_x = 8, local_size_y = 8, local_size_z = NUM_LAYERS) in;

layout (binding = 0, TEXTURE_FORMAT) writeonly uniform IMAGE_TYPE renderTarget;

layout (location = 0) uniform CLEAR_VALUE_TYPE clearValues;

void main() {
    ivec2 sceneSize = imageSize(renderTarget);
    ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);

    if (currentTexel.x < sceneSize.x && currentTexel.y < sceneSize.y) {
#if TEXTURE_DIMENSION == TEXTURE_DIMENSION_2D
        imageStore(renderTarget, currentTexel, clearValues);
#elif TEXTURE_DIMENSION == TEXTURE_DIMENSION_CUBE
        int layer = int(gl_GlobalInvocationID.z);
        imageStore(renderTarget, ivec3(currentTexel, layer), clearValues);
#endif
	}
}
