#version 460 core

#include "core/common.glsl"
#include "core/diffuse_sh.glsl"

// "An Efficient Representation for Irradiance Environment Maps" by Ravi Ramamoorthi and Pat Hanrahan.

// --------------------------------------------------------
// Definitions

// 8x8 is too big for shared memory
#define GROUP_SIZE_X 4
#define GROUP_SIZE_Y 4

#define CUBEMAP_TYPE_SKYBOX      0
#define CUBEMAP_TYPE_LIGHT_PROBE 1
//?#define CUBEMAP_TYPE CUBEMAP_TYPE_SKYBOX

#if CUBEMAP_TYPE == CUBEMAP_TYPE_SKYBOX
    #define CUBEMAP_ELEMENT vec3
#elif CUBEMAP_TYPE == CUBEMAP_TYPE_LIGHT_PROBE
    #define CUBEMAP_ELEMENT vec4
#else
    #error "CUBEMAP_TYPE was not defined"
#endif

// --------------------------------------------------------
// Layout

layout (local_size_x = GROUP_SIZE_X, local_size_y = GROUP_SIZE_Y, local_size_z = 6) in;

layout (location = 1) uniform uint cubemapSize;
layout (location = 2) uniform uint shIndex;

layout (binding = 0) uniform samplerCube inColorCubemap;
#if CUBEMAP_TYPE == CUBEMAP_TYPE_LIGHT_PROBE
layout (binding = 1) uniform samplerCube inDepthCubemap;
#endif

layout (std140, binding = 2) writeonly buffer Buffer_SH {
	SHBuffer outSH[];
};

shared CUBEMAP_ELEMENT s_perFaceL[6][9];
shared float         s_perFaceWeight[6];
shared CUBEMAP_ELEMENT s_sliceL[6][GROUP_SIZE_Y][GROUP_SIZE_X][9];
shared float         s_weight[6][GROUP_SIZE_Y][GROUP_SIZE_X];

// --------------------------------------------------------
// Shader

uint CUBEMAP_SIZE() { return cubemapSize; }
uint SH_INDEX() { return shIndex; }

CUBEMAP_ELEMENT SAMPLE_CUBEMAP_FUNCTION(vec3 dir) {
#if CUBEMAP_TYPE == CUBEMAP_TYPE_SKYBOX
    return textureLod(inColorCubemap, dir, 0).xyz;
#elif CUBEMAP_TYPE == CUBEMAP_TYPE_LIGHT_PROBE
    vec3 color = textureLod(inColorCubemap, dir, 0).xyz;
    float isSky = textureLod(inDepthCubemap, dir, 0).x > 64000.0 ? 1.0 : 0.0;
    return vec4(color, isSky);
#else
    #error "Unhandled static branch"
#endif
}

vec4 STORE_SH(CUBEMAP_ELEMENT value) {
#if CUBEMAP_TYPE == CUBEMAP_TYPE_SKYBOX
    return vec4(value, 0);
#elif CUBEMAP_TYPE == CUBEMAP_TYPE_LIGHT_PROBE
    return value;
#else
    #error "Unhandled static branch"
#endif
}

float Y00  (vec3 dir) { return 0.282095f; };
float Y1_1 (vec3 dir) { return 0.488603f * dir.y; };
float Y10  (vec3 dir) { return 0.488603f * dir.z; };
float Y11  (vec3 dir) { return 0.488603f * dir.x; };
float Y2_2 (vec3 dir) { return 1.092548f * dir.x * dir.y; };
float Y2_1 (vec3 dir) { return 1.092548f * dir.y * dir.z; };
float Y20  (vec3 dir) { return 0.315392f * (3.0f * dir.z * dir.z - 1.0f); };
float Y21  (vec3 dir) { return 1.092548f * dir.x * dir.z; };
float Y22  (vec3 dir) { return 0.546274f * (dir.x * dir.x - dir.y * dir.y); };

vec3 getDir(uint face, float u, float v) {
    if (face == 0) return normalize(vec3(1, u, v));
    else if (face == 1) return normalize(vec3(-1, u, v));
    else if (face == 2) return normalize(vec3(u, 1, v));
    else if (face == 3) return normalize(vec3(u, -1, v));
    else if (face == 4) return normalize(vec3(u, v, 1));
    else if (face == 5) return normalize(vec3(u, v, -1));
    return vec3(0);
}

void prefilter(uint face, uint x0, uint y0) {
    CUBEMAP_ELEMENT scratch[9];
    for (int i = 0; i < 9; i++) scratch[i] = CUBEMAP_ELEMENT(0);
    float wSum = 0.0;

    uint size = CUBEMAP_SIZE();
    for (uint y = y0; y < size; y += GROUP_SIZE_Y) {
        for (uint x = x0; x < size; x += GROUP_SIZE_X) {
            float u = 2.0 * (float(x) / float(size)) - 1.0;
            float v = 2.0 * (float(y) / float(size)) - 1.0;
            float tmp = 1.0 + u * u + v * v;
            float w = 4.0 / (tmp * sqrt(tmp));

            vec3 dir = getDir(face, u, v);
            CUBEMAP_ELEMENT color = w * SAMPLE_CUBEMAP_FUNCTION(dir);

            scratch[0] += color * Y00(dir);
            scratch[1] += color * Y1_1(dir);
            scratch[2] += color * Y10(dir);
            scratch[3] += color * Y11(dir);
            scratch[4] += color * Y2_2(dir);
            scratch[5] += color * Y2_1(dir);
            scratch[6] += color * Y20(dir);
            scratch[7] += color * Y21(dir);
            scratch[8] += color * Y22(dir);

            wSum += w;
        }
    }
    
    for (int i = 0; i < 9; i++) {
        s_sliceL[face][y0][x0][i] = scratch[i];
        s_weight[face][y0][x0] = wSum;
    }
    groupMemoryBarrier();
    barrier();
}

void main() {
    uvec3 tid = gl_LocalInvocationID.xyz;
    const uint face = tid.z;

    s_perFaceWeight[face] = 0.0;
    for (int i = 0; i < 9; i++) {
        s_perFaceL[face][i] = CUBEMAP_ELEMENT(0);
        s_sliceL[face][tid.y][tid.x][i] = CUBEMAP_ELEMENT(0);
        s_weight[face][tid.y][tid.x] = 0.0;
    }
    groupMemoryBarrier();
    barrier();

    prefilter(face, tid.x, tid.y);

    // Collect per-face data.
    if (tid.xy == uvec2(0, 0)) {
        CUBEMAP_ELEMENT scratch[9];
        for (int i = 0; i < 9; i++) scratch[i] = CUBEMAP_ELEMENT(0);
        float wSum = 0.0;

        for (uint y = 0; y < GROUP_SIZE_Y; y++) {
            for (uint x = 0; x < GROUP_SIZE_X; x++) {
                scratch[0] += s_sliceL[face][y][x][0];
                scratch[1] += s_sliceL[face][y][x][1];
                scratch[2] += s_sliceL[face][y][x][2];
                scratch[3] += s_sliceL[face][y][x][3];
                scratch[4] += s_sliceL[face][y][x][4];
                scratch[5] += s_sliceL[face][y][x][5];
                scratch[6] += s_sliceL[face][y][x][6];
                scratch[7] += s_sliceL[face][y][x][7];
                scratch[8] += s_sliceL[face][y][x][8];

                wSum += s_weight[face][y][x];
            }
        }
        groupMemoryBarrier();
        barrier();

        s_perFaceWeight[face] = wSum;
        for (int i = 0; i < 9; i++) s_perFaceL[face][i] = scratch[i];
        groupMemoryBarrier();
        barrier();
    }

    if (tid == uvec3(0, 0, 0)) {
        float wSum = 0.0;
        wSum += s_perFaceWeight[0];
        wSum += s_perFaceWeight[1];
        wSum += s_perFaceWeight[2];
        wSum += s_perFaceWeight[3];
        wSum += s_perFaceWeight[4];
        wSum += s_perFaceWeight[5];

        float norm = 4.0 * PI / wSum;

        for (int i = 0; i < 9; i++) {
            CUBEMAP_ELEMENT L = CUBEMAP_ELEMENT(0);
            L += s_perFaceL[0][i];
            L += s_perFaceL[1][i];
            L += s_perFaceL[2][i];
            L += s_perFaceL[3][i];
            L += s_perFaceL[4][i];
            L += s_perFaceL[5][i];

            outSH[SH_INDEX()].Ls[i] = STORE_SH(L * norm);
        }
    }
}
