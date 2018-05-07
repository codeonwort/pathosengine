// Normal mapping + shadow mapping for forward renderer.
// TODO: port to deferred renderer.

//////////////////////////////////////////////////////////////////
// Vertex shader

#version 430 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;
layout (location = 1) in vec2 uv;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
    vec3 eye_tangent; // for normal mapping
    float light_distance;
    vec3 light_tangent; // for normal mapping
    vec3 normal;
    vec3 omniShadowCoord;
    vec4 shadowMapCoord[2];
    vec2 uv;
} vs_out;

uniform mat4 modelTransform;
uniform mat4 mvpTransform;
uniform vec3 eyeDir_camera;
uniform vec3 lightPos_camera;
uniform mat3 mvTransform3x3;
uniform mat4 mvTransform4x4;
uniform uint numDirLights;
uniform mat4 depthMVP[2];

void main() {
    vec3 tangent_camera = mvTransform3x3 * tangent;
    vec3 bitangent_camera = mvTransform3x3 * bitangent;
    vec3 normal_camera = mvTransform3x3 * normal;
    mat3 TBN = transpose(mat3(tangent_camera, bitangent_camera, normal_camera));
    vec3 position_camera = vec3(mvTransform4x4 * vec4(position, 1.0f));
    vec3 lightDir = lightPos_camera - position_camera;
    vs_out.light_distance = dot(lightDir, lightDir);
    vs_out.light_tangent = TBN * normalize(lightDir);
    vs_out.eye_tangent = TBN * eyeDir_camera;
    for (uint i = 0; i < numDirLights; ++i) {
        vs_out.shadowMapCoord[i] = depthMVP[i] * vec4(position, 1);
    }
    vs_out.omniShadowCoord = vec3(modelTransform * vec4(position, 1.0f));
    vs_out.normal = normalize((modelTransform * vec4(normal, 0.0)).xyz);
    vs_out.uv = uv;
    gl_Position = mvpTransform * vec4(position, 1.0);
}

//////////////////////////////////////////////////////////////////
// Fragment shader

#version 430 core

uniform sampler2D diffuseSampler;
uniform sampler2D normalSampler;
uniform uint numDirLights;
uniform uint numPointLights;
uniform sampler2DShadow depthSampler[2];
uniform samplerCubeShadow omniShadow_depthSampler[4];
uniform float f_plus_n;
uniform float f_minus_n;
uniform float f_mult_n;
uniform vec3 dirLightDirs[2];
uniform vec3 dirLightColors[2];
uniform vec3 pointLightPos[4];
uniform vec3 pointLightColors[4];

in VS_OUT {
    vec3 eye_tangent;
    float light_distance;
    vec3 light_tangent;
    vec3 normal;
    vec3 omniShadowCoord;
    vec4 shadowMapCoord[2];
    vec2 uv;
} fs_in;

out vec4 color;

void main() {
    float visibility = 1.0f;

    // directional shadow
    vec3 sm_normal = normalize(fs_in.normal);
    for(uint i=0; i<numDirLights; ++i) {
        float cosTheta = clamp(dot(sm_normal, -dirLightDirs[i]), 0.0, 1.0);
        float bias = clamp(0.005 * tan(acos(cosTheta)), 0.0, 0.1);
        float w = fs_in.shadowMapCoord[i].w;
        float depth_test = texture(depthSampler[i], vec3(fs_in.shadowMapCoord[i].xy / w, (fs_in.shadowMapCoord[i].z - bias) / w));
        if(depth_test < .5) visibility *= 0.2;
    }

    // omnidirectional shadow
    uint omni_count = 0;
    for (uint i = 0; i < numPointLights; ++i) {
        vec3 dir = fs_in.omniShadowCoord - pointLightPos[i];
        float cosTheta = clamp(dot(normalize(fs_in.normal), -normalize(dir)), 0.0, 1.0);
        float bias = clamp(0.005 * tan(acos(cosTheta)), 0.0, 0.1);
        float localZ = max(abs(dir.x), max(abs(dir.y), abs(dir.z)));
        localZ = f_plus_n / f_minus_n - (2 * f_mult_n) / f_minus_n / localZ;
        localZ = (localZ - bias + 1) * 0.5;
        float depth_test = texture(omniShadow_depthSampler[i], vec4(normalize(dir), localZ));
        if(depth_test > .5) ++omni_count;
    }
    if (omni_count == 0) visibility *= 0.5;

    // normal mapping
    vec3 norm = normalize(texture2D(normalSampler, fs_in.uv).rgb * 2.0 - 1.0);

    vec3 diffuseTerm = vec3(0.0);
    vec3 specularTerm = vec3(0.0);
    vec3 halfVector = normalize(fs_in.light_tangent + fs_in.eye_tangent);
    float lambert = clamp(dot(norm, normalize(fs_in.light_tangent)), 0.0, 1.0);
    float attenuation = 1 / (1 + 0.0003 * fs_in.light_distance);

    diffuseTerm = (attenuation * visibility * lambert) * vec3(1.0);
    specularTerm = (visibility * pow(max(dot(norm, halfVector), 0.0), 128)) * vec3(1.0);

    vec4 final_diffuse = vec4(diffuseTerm, 1.0) * texture2D(diffuseSampler, fs_in.uv);
    vec4 final_specular = 0.8 * vec4(specularTerm, 1.0);

    // final output
    color = visibility * (final_diffuse + final_specular);
}