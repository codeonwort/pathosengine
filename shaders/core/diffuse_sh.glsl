//?#version 460 core

// #todo-rhi: Align 16 bytes or use float array?
// 1. vec4 * 9 = 144 bytes, throw away all w channels. (Both std140 and std430 works)
// 2. float[28] = 112 bytes, only last element is unused. (Only std430 works)
// glslang disassembly is shorter for option 1, but not sure if it's also more performant.
struct SHBuffer {
    vec4 Ls[9]; // w maybe not used. See comute_diffuse_sh.glsl
};

vec4 evaluateSH(SHBuffer shBuffer, vec3 dir) {
    const float c1 = 0.429043f;
    const float c2 = 0.511664f;
    const float c3 = 0.743125f;
    const float c4 = 0.886227f;
    const float c5 = 0.247708f;

    const vec4 L00  = shBuffer.Ls[0];
    const vec4 L1_1 = shBuffer.Ls[1];
    const vec4 L10  = shBuffer.Ls[2];
    const vec4 L11  = shBuffer.Ls[3];
    const vec4 L2_2 = shBuffer.Ls[4];
    const vec4 L2_1 = shBuffer.Ls[5];
    const vec4 L20  = shBuffer.Ls[6];
    const vec4 L21  = shBuffer.Ls[7];
    const vec4 L22  = shBuffer.Ls[8];

    float x = dir.x;
    float y = dir.y;
    float z = dir.z;

    vec4 E = vec4(0.0f);
    E += c1 * L22 * (x * x - y * y) + c3 * L20 * z * z + c4 * L00 - c5 * L20;
    E += 2 * c1 * (L2_2 * x * y + L21 * x * z + L2_1 * y * z);
    E += 2 * c2 * (L11 * x + L1_1 * y + L10 * z);
    return E;
}
