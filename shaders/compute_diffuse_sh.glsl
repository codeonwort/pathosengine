#version 460 core

#include "core/common.glsl"

// "An Efficient Representation for Irradiance Environment Maps" by Ravi Ramamoorthi and Pat Hanrahan.

// --------------------------------------------------------
// Definitions

#define GROUP_SIZE_X 8
#define GROUP_SIZE_Y 8

struct SHBuffer {
    vec4 Ls[9]; // w not used
};

// --------------------------------------------------------
// Layout

layout (local_size_x = GROUP_SIZE_X, local_size_y = GROUP_SIZE_Y, local_size_z = 6) in;

layout (location = 1) uniform int cubemapSize;

layout (binding = 0) uniform samplerCube inCubemap;

layout (std140, binding = 2) writeonly buffer Buffer_SH {
	SHBuffer outSH;
};

shared vec3 s_perFaceL[6][9];
shared float s_perFaceWeight[6];
shared vec3 s_sliceL[GROUP_SIZE_Y][GROUP_SIZE_X][9];
shared float s_weight[6][GROUP_SIZE_Y][GROUP_SIZE_X];

// --------------------------------------------------------
// Shader

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
    vec3 scratch[9];
    for (int i = 0; i < 9; i++) scratch[i] = vec3(0);
    float wSum = 0.0;

    for (uint y = y0; y < cubemapSize; y += GROUP_SIZE_Y) {
        for (uint x = x0; x < cubemapSize; x += GROUP_SIZE_X) {
            float u = 2.0 * (float(x) / float(cubemapSize)) - 1.0;
            float v = 2.0 * (float(y) / float(cubemapSize)) - 1.0;
            float tmp = 1.0 + u * u + v * v;
            float w = 4.0 / (tmp * sqrt(tmp));

            vec3 dir = getDir(face, u, v);
            vec3 sky = w * textureLod(inCubemap, dir, 0).xyz;

            scratch[0] += sky * Y00(dir);
            scratch[1] += sky * Y1_1(dir);
            scratch[2] += sky * Y10(dir);
            scratch[3] += sky * Y11(dir);
            scratch[4] += sky * Y2_2(dir);
            scratch[5] += sky * Y2_1(dir);
            scratch[6] += sky * Y20(dir);
            scratch[7] += sky * Y21(dir);
            scratch[8] += sky * Y22(dir);

            wSum += w;
        }
    }

    for (int i = 0; i < 9; i++) {
        s_sliceL[y0][x0][i] = scratch[i];
        s_weight[face][y0][x0] = wSum;
    }
}

vec3 evaluateSH(SHBuffer shBuffer, vec3 dir) {
    const float c1 = 0.429043f;
    const float c2 = 0.511664f;
    const float c3 = 0.743125f;
    const float c4 = 0.886227f;
    const float c5 = 0.247708f;

    const vec3 L00  = shBuffer.Ls[0].xyz;
    const vec3 L1_1 = shBuffer.Ls[1].xyz;
    const vec3 L10  = shBuffer.Ls[2].xyz;
    const vec3 L11  = shBuffer.Ls[3].xyz;
    const vec3 L2_2 = shBuffer.Ls[4].xyz;
    const vec3 L2_1 = shBuffer.Ls[5].xyz;
    const vec3 L20  = shBuffer.Ls[6].xyz;
    const vec3 L21  = shBuffer.Ls[7].xyz;
    const vec3 L22  = shBuffer.Ls[8].xyz;

    float x = dir.x;
    float y = dir.y;
    float z = dir.z;

    vec3 E = vec3(0.0f);
    E += c1 * L22 * (x * x - y * y) + c3 * L20 * z * z + c4 * L00 - c5 * L20;
    E += 2 * c1 * (L2_2 * x * y + L21 * x * z + L2_1 * y * z);
    E += 2 * c2 * (L11 * x + L1_1 * y + L10 * z);
    return E;
}

void main() {
    uvec3 tid = gl_LocalInvocationID.xyz;
    const uint face = tid.z;

    s_perFaceWeight[face] = 0.0;
    for (int i = 0; i < 9; i++) {
        s_perFaceL[face][i] = vec3(0);
        s_sliceL[tid.y][tid.x][i] = vec3(0);
        s_weight[face][tid.y][tid.x] = 0.0;
    }
    memoryBarrierShared();

    prefilter(face, tid.x, tid.y);
    memoryBarrierShared();

    // Collect per-face data.
    if (tid.xy == uvec2(0, 0)) {
        vec3 scratch[9];
        for (int i = 0; i < 9; i++) scratch[i] = vec3(0);
        float wSum = 0.0;

        for (uint y = 0; y < GROUP_SIZE_Y; y++) {
            for (uint x = 0; x < GROUP_SIZE_X; x++) {
                scratch[0] += s_sliceL[y][x][0];
                scratch[1] += s_sliceL[y][x][1];
                scratch[2] += s_sliceL[y][x][2];
                scratch[3] += s_sliceL[y][x][3];
                scratch[4] += s_sliceL[y][x][4];
                scratch[5] += s_sliceL[y][x][5];
                scratch[6] += s_sliceL[y][x][6];
                scratch[7] += s_sliceL[y][x][7];
                scratch[8] += s_sliceL[y][x][8];

                wSum += s_weight[face][y][x];
            }
        }
        s_perFaceWeight[face] = wSum;
        for (int i = 0; i < 9; i++) s_perFaceL[face][i] = scratch[i];
    }
    memoryBarrierShared();

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
            vec3 L = vec3(0);
            L += s_perFaceL[0][i];
            L += s_perFaceL[1][i];
            L += s_perFaceL[2][i];
            L += s_perFaceL[3][i];
            L += s_perFaceL[4][i];
            L += s_perFaceL[5][i];

            outSH.Ls[i] = vec4(L, 0) * norm;
        }
    }
}
