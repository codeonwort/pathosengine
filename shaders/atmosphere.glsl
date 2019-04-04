#version 460 core

#include "deferred_common.glsl"

layout (location = 0) out vec4 color;

layout (std140, binding = 1) uniform UBO_AtmosphereScattering {
	vec4 sunParams;
} ubo;

//////////////////////////////////////////////////////////////////////////
// Constants
#define MAGIC_RAYLEIGH        1.0
#define MAGIC_MIE             0.3

#define PI                    3.14159265359

// Unit: meters
#define SUN_DISTANCE          1.496e11
#define SUN_RADIUS            6.9551e8
#define EARTH_RADIUS          6.36e6
#define ATMOSPHERE_RADIUS     6.42e6
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
	vec3(0., 0., 0.), // normal
	vec3(0., 0., 0.) // origin
);

ray_t get_primary_ray(
	in vec3 cam_local_point,
	inout vec3 cam_origin,
	inout vec3 cam_look_at)
{
	vec3 fwd = normalize(cam_look_at - cam_origin);
	vec3 up = vec3(0, 1, 0);
	vec3 right = cross(up, fwd);
	up = cross(fwd, right);

    ray_t r;
    r.origin = cam_origin;
    r.direction = normalize(fwd + up * cam_local_point.y - right * cam_local_point.x);
	return r;
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

// Radiance of sun before hitting the atmosphere
vec3 sunImage(ray_t camera, vec3 sunDir)
{
	float sunSize = ubo.sunParams.x;
	vec3 sunIntensity = vec3(ubo.sunParams.y);

    float threshold = asin(SUN_RADIUS / SUN_DISTANCE);
    float angle = acos(dot(camera.direction, -sunDir));
    if(angle <= threshold * sunSize)
    {
        return sunIntensity;
    }
    return vec3(0.0);
}

vec3 scene(ray_t camera, vec3 sunDir)
{
    hit_t hit = no_hit;
    intersect_sphere(camera, atmosphere, hit);

    vec3 AtmosphereScattering = vec3(0.0);
    bool isGround = false;
    
    const int numSteps = 16;
    const int inscatSteps = 8;
    
    float mu = dot(-sunDir, camera.direction);
    vec3 Sun = vec3(ubo.sunParams.y);
    vec3 T = vec3(0.0);
    
    vec3 P = camera.origin;
    float seg = hit.t / float(numSteps);
    vec3 P_step = camera.direction * seg;
    
    // from eye to the outer end of atmosphere
    for(int i=0; i<numSteps; ++i)
    {
        float height = length(P) - EARTH_RADIUS;
        if(height < 0.0)
        {
            isGround = true;
            break;
        }
        
        // optical depth
        T += seg * (BetaR * exp(-height / Hr));
        T += seg * (BetaM * exp(-height / Hm));
        
        // single scattering
        hit_t hit2 = no_hit;
        ray_t ray2 = ray_t(P, -sunDir);
        intersect_sphere(ray2, atmosphere, hit2);
        
        float segLight = hit2.t / float(inscatSteps);
        vec3 PL_step = ray2.direction * segLight;
        vec3 PL = P;
        
        vec3 TL = vec3(0.0);
        bool applyScattering = true;
        for(int j=0; j<inscatSteps; ++j)
        {
            float height2 = length(PL) - EARTH_RADIUS;
            if(height2 < 0.0)
            {
                applyScattering = false;
                break;
            }
            
            TL += segLight * BetaR * exp(-height2 / Hr);
        	TL += segLight * BetaM * exp(-height2 / Hm);
            
            PL += PL_step;
        }
        if(applyScattering)
        {
            TL = exp(-TL);

            vec3 SingleScattering = vec3(0.0);
            // scattering = transmittance * scattering_coefficient * phase * radiance
            SingleScattering += MAGIC_RAYLEIGH * seg * exp(-T) * (BetaR * exp(-height / Hr)) * phaseR(mu) * (TL * Sun);
            SingleScattering += MAGIC_MIE * seg * exp(-T) * (BetaM * exp(-height / Hm)) * phaseM(mu) * (TL * Sun);
            AtmosphereScattering += SingleScattering;
        }
        
        P += P_step;
    }
    
    // Just magic number
    if(isGround)
    {
		return vec3(0.0);
        //float r = 1.0 - 1.0 / (1.0 + 0.000001 * length(hit.origin - camera.origin));
        //return vec3(r, 0.4, 0.2);
    }
    
    T = exp(-T);
    
    // Zero scattering
	vec3 L0 = T * sunImage(camera, sunDir);
    AtmosphereScattering += L0;
    
    return AtmosphereScattering;
}

vec3 viewDirection() {
	vec2 uv = gl_FragCoord.xy / uboPerFrame.screenResolution.xy;
	vec3 P = vec3(2.0 * uv - 1.0, 0.0);
    P.x *= uboPerFrame.screenResolution.x / uboPerFrame.screenResolution.y;
    P.z = -(1.0 / tan(uboPerFrame.zRange.z * 0.5));
	P = normalize(P);

    mat3 camera_transform = mat3(uboPerFrame.inverseViewTransform);
	vec3 ray_forward = camera_transform * P;

    return ray_forward;
}

void main() {
	ray_t eye_ray;
	eye_ray.origin = vec3(0.0, EARTH_RADIUS + 1.84, 0.0);
	eye_ray.direction = viewDirection();

	vec3 sunDir = uboPerFrame.dirLightDirs[0];
    
    color = vec4(scene(eye_ray, sunDir), 1.0);
}
