#version 460 core

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance

const float PI = 3.14159265359;

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

out vec4 outIrradiance;

layout (binding = 0) uniform samplerCube envMap;

#if ONV_ENCODING
float ONVSaturate ( float x ) { return max(0.0, min(1.0, x)); }
vec2 ONVOctWrap( vec2 v ) {
    vec2 w = 1.0 - abs( v.yx );
    if (w.x < 0.0) w.x = -w.x;
    if (w.y < 0.0) w.y = -w.y;
    return w;
}
vec2 ONVEncode( vec3 n ) {
    n /= ( abs( n.x ) + abs( n.y ) + abs( n.z ) );
    n.xy = n.z >= 0.0 ? n.xy : ONVOctWrap( n.xy );
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}
vec3 ONVDecode( vec2 f ) {
    f = f * 2.0 - 1.0;
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3( f.x, f.y, 1.0 - abs( f.x ) - abs( f.y ) );
    float t = ONVSaturate( -n.z );
    n.x += n.x >= 0.0 ? -t : t;
    n.y += n.y >= 0.0 ? -t : t;
    return normalize( n );
}
#endif // ONV_ENCODING

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

            irradiance += texture(envMap, sampleVec).rgb * cosTheta * sinTheta;
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    outIrradiance = vec4(irradiance, 1.0);
}

#endif // FRAGMENT_SHADER
