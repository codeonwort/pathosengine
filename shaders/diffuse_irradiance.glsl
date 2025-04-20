#version 460 core

#include "core/common.glsl"

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
// "An Efficient Representation for Irradiance Environment Maps" by Ravi Ramamoorthi and Pat Hanrahan.

////////////////////////////////////////////////////////////

#if COMPUTE_SHADER

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (location = 1) uniform int cubemapSize;

layout (binding = 0) uniform samplerCube inCubemap;

struct SHBuffer {
    vec4 Ls[9]; // w not used
};

layout (std140, binding = 2) writeonly buffer Buffer_SH {
	SHBuffer outSH;
};

float Y00  (vec3 dir) { return 0.282095f; };
float Y1_1 (vec3 dir) { return 0.488603f * dir.y; };
float Y10  (vec3 dir) { return 0.488603f * dir.z; };
float Y11  (vec3 dir) { return 0.488603f * dir.x; };
float Y2_2 (vec3 dir) { return 1.092548f * dir.x * dir.y; };
float Y2_1 (vec3 dir) { return 1.092548f * dir.y * dir.z; };
float Y20  (vec3 dir) { return 0.315392f * (3.0f * dir.z * dir.z - 1.0f); };
float Y21  (vec3 dir) { return 1.092548f * dir.x * dir.z; };
float Y22  (vec3 dir) { return 0.546274f * (dir.x * dir.x - dir.y * dir.y); };

vec3 getDir(int face, float u, float v) {
    if (face == 0) return normalize(vec3(1, u, v));
    else if (face == 1) return normalize(vec3(-1, u, v));
    else if (face == 2) return normalize(vec3(u, 1, v));
    else if (face == 3) return normalize(vec3(u, -1, v));
    else if (face == 4) return normalize(vec3(u, v, 1));
    else if (face == 5) return normalize(vec3(u, v, -1));
    return vec3(0);
}

void prefilter() {
    vec3 scratch[9];
    for (int i = 0; i < 9; i++) scratch[i] = vec3(0);
    float wSum = 0.0;

    for (int face = 0; face < 6; face++) {
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
    }

    float norm = 4.0 * PI / wSum;
    for (int i = 0; i < 9; i++) {
        outSH.Ls[i] = vec4(scratch[i] * norm, 0);
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
    prefilter();
}

#endif // COMPUTE_SHADER

////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 inPosition;

out VS_OUT {
    vec3 posL;
} interpolants;

layout (location = 0) uniform mat4 transform;

void main() {
    interpolants.posL = inPosition;
    gl_Position = transform * vec4(inPosition, 1.0);
}

#endif // VERTEX_SHADER

////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

#ifndef ONV_ENCODING
    #define ONV_ENCODING 0
#endif

#if ONV_ENCODING
in VS_OUT {
	vec2 screenUV;
} interpolants;
#else
in VS_OUT {
    vec3 posL;
} interpolants;
#endif

layout (binding = 0) uniform samplerCube inRadianceCubemap;
#if ONV_ENCODING
layout (binding = 1) uniform samplerCube inDepthCubemap;
#endif

layout (location = 0) out vec4 outIrradiance;
#if ONV_ENCODING
layout (location = 1) out float outLinearDepth;
#endif

void main() {
#if ONV_ENCODING
    vec3 dir = ONVDecode(interpolants.screenUV);
#else
    vec3 dir = normalize(interpolants.posL);
#endif

    vec3 irradiance = vec3(0.0);
#if ONV_ENCODING
    float skyVisibility = 0.0f;
#endif

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, dir);
    up         = cross(dir, right);

    // #todo: Loop count ~= 7781
    // Should optimize this. Maybe https://www.activision.com/cdn/research/paper_egsr.pdf
    float sampleDelta = 0.05;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            // Spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
            // Tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * dir;

            irradiance += texture(inRadianceCubemap, sampleVec).rgb * cosTheta * sinTheta;
#if ONV_ENCODING
            float isSky = texture(inDepthCubemap, sampleVec).r > 64000.0 ? 1.0 : 0.0;
            skyVisibility += isSky * cosTheta * sinTheta;
#endif
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
#if ONV_ENCODING
    skyVisibility = PI * skyVisibility * (1.0 / float(nrSamples));
#endif

#if ONV_ENCODING
    outIrradiance = vec4(irradiance, skyVisibility);
    outLinearDepth = texture(inDepthCubemap, dir).r;
#else
    outIrradiance = vec4(irradiance, 1.0);
#endif
}

#endif // FRAGMENT_SHADER
