#version 460 core

#include "core/common.glsl"

// "An Efficient Representation for Irradiance Environment Maps" by Ravi Ramamoorthi and Pat Hanrahan.

// --------------------------------------------------------
// Definitions

struct SHBuffer {
    vec4 Ls[9]; // w not used
};

// --------------------------------------------------------
// Layout

layout (local_size_x = 1, local_size_y = 1, local_size_z = 6) in;

layout (location = 1) uniform int cubemapSize;

layout (binding = 0) uniform samplerCube inCubemap;

layout (std140, binding = 2) writeonly buffer Buffer_SH {
	SHBuffer outSH;
};

shared vec3 s_faceL[6][9];

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

void prefilter(uint face) {
    vec3 scratch[9];
    for (int i = 0; i < 9; i++) scratch[i] = vec3(0);
    float wSum = 0.0;

    for (int y = 0; y < cubemapSize; y++) {
        for (int x = 0; x < cubemapSize; x++) {
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

    float norm = 4.0 * PI / wSum;
    for (int i = 0; i < 9; i++) {
        s_faceL[face][i] = scratch[i] * norm;
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
    const uint face = gl_LocalInvocationID.z;

    for (int i = 0; i < 9; i++) {
        s_faceL[face][i] = vec3(0);
    }
    memoryBarrierShared();

    prefilter(face);
    memoryBarrierShared();

    if (gl_LocalInvocationID.x == 0) {
        for (int i = 0; i < 9; i++) {
            vec3 L = vec3(0);
            L += s_faceL[0][i];
            L += s_faceL[1][i];
            L += s_faceL[2][i];
            L += s_faceL[3][i];
            L += s_faceL[4][i];
            L += s_faceL[5][i];

            outSH.Ls[i] = vec4(L, 0);
        }
    }
}
