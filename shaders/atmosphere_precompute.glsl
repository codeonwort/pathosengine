#version 450 core

#include "deferred_common.glsl"

#ifndef PRECOMPUTE_TRANSMITTANCE
    #define PRECOMPUTE_TRANSMITTANCE 0
#endif

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (location = 0) out vec4 outResult;

//////////////////////////////////////////////////////////////////////////
// Constants
#define PI                    3.14159265359
// Unit: meters
#define EARTH_RADIUS          6.36e6
#define ATMOSPHERE_RADIUS     6.42e6
#define GROUND_EPSILON        (1.84) // Avoid collision to ground at uv.x = 0
#define MAX_ALTITUDE          (ATMOSPHERE_RADIUS - EARTH_RADIUS - GROUND_EPSILON)
#define Hr                    7.994e3
#define Hm                    1.2e3
// Unit: 1 / meters
#define BetaR                 vec3(5.8e-6, 13.5e-6, 33.1e-6)
#define BetaM                 vec3(21e-6)

//////////////////////////////////////////////////////////////////////////
// Ray, sphere code from https://www.shadertoy.com/view/XtBXDw
struct ray_t {
	vec3 origin;
	vec3 direction;
};

struct sphere_t {
	vec3 origin;
	float radius;
};

struct hit_t {
	float t;
	vec3 normal;
	vec3 origin;
};

#define max_dist 1e8
hit_t no_hit = hit_t(
	float(max_dist + 1e1), // 'infinite' distance
	vec3(0.0), // normal
	vec3(0.0) // origin
);

bool validHit(hit_t hit) {
    return 0.0 < hit.t && hit.t < max_dist;
}

void intersect_sphere(
	in ray_t ray,
	in sphere_t sphere,
	inout hit_t hit
){
	vec3 rc = sphere.origin - ray.origin;
	float radius2 = sphere.radius * sphere.radius;
	float tca = dot(rc, ray.direction);
//	if (tca < 0.) return;

	float d2 = dot(rc, rc) - tca * tca;
	if (d2 > radius2)
		return;

	float thc = sqrt(radius2 - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	if (t0 < 0.) t0 = t1;
	if (t0 > hit.t)
		return;

	vec3 impact = ray.origin + ray.direction * t0;

	hit.t = t0;
	hit.origin = impact;
	hit.normal = (impact - sphere.origin) / sphere.radius;
}

//////////////////////////////////////////////////////////////////////////

sphere_t earth = sphere_t(vec3(0.0), EARTH_RADIUS);
sphere_t atmosphere = sphere_t(vec3(0.0), ATMOSPHERE_RADIUS);

float phaseR(float cosTheta)
{
    return 3.0 / (16.0 * PI) * (1.0 + cosTheta * cosTheta);
}
float phaseM(float t)
{
    const float g = 0.76;
    float gg = g * g;
    float tt = t * t;
    float gt = g * t;
    
    float num = 3.0 * (1.0 - gg) * (1.0 + tt);
	float denom = (8.0 * PI) * (2.0 + gg) * pow(1.0 + gg - 2.0 * gt, 1.5);
    return num / denom;
}

// uv.x : [0, 1] -> [EARTH_RADIUS, ATMOSPHERE_RADIUS]
// uv.y : [0, 1] -> [0, PI]
vec3 precomputeTransmittance(vec2 uv) {
    const int NUM_INTEG_STEPS = 512;

    // Due to spherical symmetry we only care about case of +X axis.
    float mu = PI * uv.y; // View-zenith angle
    float h = mix(EARTH_RADIUS + GROUND_EPSILON, ATMOSPHERE_RADIUS, uv.x);
    vec3 x = vec3(h, 0.0, 0.0);
    vec3 v = vec3(cos(mu), sin(mu), 0.0); // View direction
    
    hit_t hit_atmosphere = no_hit;
    hit_t hit_earth = no_hit;
    ray_t ray = ray_t(x, v);
    intersect_sphere(ray, atmosphere, hit_atmosphere);
    intersect_sphere(ray, earth, hit_earth);
    float rayT = 0.0;
    if (validHit(hit_atmosphere) && validHit(hit_earth)) {
        if (hit_earth.t < hit_atmosphere.t) {
            return vec3(0.0);
        }
        rayT = min(hit_atmosphere.t, hit_earth.t);
    } else if (validHit(hit_atmosphere)) {
        rayT = hit_atmosphere.t;
    } else if (validHit(hit_earth)) {
        rayT = hit_earth.t;
        return vec3(0.0);
    } else {
        // Cannot happen; debug color
        return vec3(0.0, 10000.0, 0.0);
    }
    vec3 x0 = x + v * rayT;

    vec3 opticalDepth = vec3(0.0);
    vec3 dx = (x0 - x) / float(NUM_INTEG_STEPS);
    float dx_length = rayT / float(NUM_INTEG_STEPS);

#define ZERO_T_ON_HIT_GROUND 1
    bool isGround = false;

    for (int i = 0; i < NUM_INTEG_STEPS; i++) {
#if ZERO_T_ON_HIT_GROUND
        float height = length(x) - EARTH_RADIUS;
        if (height < 0.0) {
            isGround = true;
            break;
        }
#else
        //float height = max(0.0, length(x) - EARTH_RADIUS);
        float height = length(x) - EARTH_RADIUS;
#endif

        // Accumulate optical depth (converted to transmittance at end of loop)
        opticalDepth += (BetaR * exp(-height / Hr)) * dx_length;
        opticalDepth += (BetaM * exp(-height / Hm)) * dx_length;

        x += dx;
    }

    vec3 T = exp(-opticalDepth);

#if ZERO_T_ON_HIT_GROUND
    if (isGround) {
        T = vec3(0.0);
    }
#endif

    return T;
}

void main() {
    vec2 uv = fs_in.screenUV;
    outResult = vec4(0.0, 0.0, 0.0, 0.0);

#if PRECOMPUTE_TRANSMITTANCE
    outResult = vec4(precomputeTransmittance(uv), 1.0);
#endif

//#if PRECOMPUTE_REFLECTANCE
    //#todo
//#endif

//#if PRECOMPUTE_SCATTERING
    //#todo
//#endif
}
