#version 450 core

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance

////////////////////////////////////////////////////////////

#if VERTEX_SHADER
layout (location = 0) in vec3 position;

out vec3 local_position;

layout (location = 0) uniform mat4 transform;

void main() {
    local_position = position;
    gl_Position = transform * vec4(local_position, 1.0);
}
#endif

////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER
in vec3 local_position;
out vec4 out_color;

layout (binding = 0) uniform samplerCube envMap;

const float PI = 3.14159265359;

void main() {
    vec3 normal = normalize(local_position);
    vec3 irradiance = vec3(0.0);

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up         = cross(normal, right);

    float sampleDelta = 0.025;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += texture(envMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    out_color = vec4(irradiance, 1.0);
}
#endif
