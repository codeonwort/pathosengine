#version 460 core

#include "../core/common.glsl"

// --------------------------------------------------------
// Layout

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (location = 1) uniform uvec4 tileCoordAndSize;

layout (binding = 0) uniform samplerCube inDepthCubemap;
layout (binding = 0, r16f) uniform writeonly image2D outDepthAtlas;

void main() {
    if (gl_GlobalInvocationID.x < tileCoordAndSize.z && gl_GlobalInvocationID.y < tileCoordAndSize.w) {
        float u = (float(gl_GlobalInvocationID.x) + 0.5) / float(tileCoordAndSize.z);
        float v = (float(gl_GlobalInvocationID.y) + 0.5) / float(tileCoordAndSize.w);
        vec2 uv = vec2(u, v);

        vec3 dir = ONVDecode(uv);
        float linearDepth = texture(inDepthCubemap, dir).x;

        ivec2 coord = ivec2(tileCoordAndSize.xy + gl_GlobalInvocationID.xy);
        imageStore(outDepthAtlas, coord, vec4(linearDepth, 0, 0, 0));
    }
}
