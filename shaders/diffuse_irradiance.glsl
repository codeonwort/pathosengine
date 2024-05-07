#version 460 core

#include "core/common.glsl"

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance

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

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, dir);
    up         = cross(dir, right);

    // #todo: Loop count ~= 15562
    // Should optimize this.
    float sampleDelta = 0.025;
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
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    outIrradiance = vec4(irradiance, 1.0);
#if ONV_ENCODING
    outLinearDepth = texture(inDepthCubemap, dir).r;
#endif
}

#endif // FRAGMENT_SHADER
