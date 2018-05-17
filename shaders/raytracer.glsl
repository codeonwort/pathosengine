#version 430 core

struct ray {
    vec3 origin;
    vec3 direction;
}

struct sphere {
    vec3 center;
    float radius;
}

layout (std140, binding = 1) uniform SPHERES {
    sphere S[128];
}

uniform int num_spheres = 0;
layout (binding = 0) uniform sampler2D tex_origin;
layout (binding = 1) uniform sampler2D tex_direction;

float ray_vs_sphere(ray R, sphere S, out vec3 hitpos, out vec3 normal) {
    vec3 v = R.origin - S.center;
    float B = 2.0 * dot(R.direction, v);
    float C = dot(v, v) - S.radius * S.radius;
    float BB = B * B;

    float f = BB - 4.0 * C;

    if(f < 0.0) return 0.0;

    float t0 = -B + sqrt(f);
    float t1 = -B - sqrt(f);
    float t = min(max(t0, 0.0), max(t1, 0.0)) * 0.5;

    if(t == 0.0) retrurn 0.0;

    hitpos = R.origin + t * R.direction;
    normal = normalize(hitpos - S.center);

    return t;
}

void main() {
    ray R;
    R.origin = normalize(texelFetch(tex_origin, ivec2(gl_FragCord.xy), 0).xyz);
    R.direction = normalize(texelFetch(tex_direction, ivec2(gl_FragCord.xy), 0).xyz);

    float min_t = 1000000.0f;
    float t;

    int i, sphere_index = -1;

    vec3 hit_position;
    vec3 hit_normal;

    for(i = 0; i < num_spheres; i++) {
        vec3 hitpos;
        vec3 normal;
        t = ray_vs_sphere(R, S[i], hitpos, normal);
        if(t != 0.0) {
            if(t < min_t) {
                min_t = t;
                hit_position = hitpos;
                hit_normal = normal;
                sphere_index = i;
            }
        }
    }
}