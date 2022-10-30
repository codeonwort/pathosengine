//?#version 460 core

// #todo: atmosphere shaders should use this.

//////////////////////////////////////////////////////////////////////////
// Ray, sphere code from https://www.shadertoy.com/view/XtBXDw

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	vec3 origin;
	float radius;
};

struct HitResult {
	float t;
	vec3 normal;
	vec3 origin;
};

#define MAX_HIT_DISTANCE 1e8
HitResult NO_HIT_RESULT = HitResult(
	float(MAX_HIT_DISTANCE + 1e1), // 'infinite' distance
	vec3(0.0, 0.0, 0.0),           // normal
	vec3(0.0, 0.0, 0.0)            // origin
);

bool isInvalidHit(HitResult hit) {
	return hit.t < 0.0 || hit.t > MAX_HIT_DISTANCE;
}

// #todo: Ray Tracing Gems - Chapter 7. Precision Improvements for Ray/Sphere Intersection
// (not adopted due to no visual differernce)
HitResult hit_ray_sphere(Ray ray, Sphere sphere) {
	HitResult hit = NO_HIT_RESULT;

	vec3 rc = sphere.origin - ray.origin;
	float radius2 = sphere.radius * sphere.radius;
	float tca = dot(rc, ray.direction);
//	if (tca < 0.) return;

	float d2 = dot(rc, rc) - tca * tca;
	if (d2 > radius2) return hit;

	float thc = sqrt(radius2 - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	if (t0 < 0.) t0 = t1;
	if (t0 > hit.t) return hit;

	vec3 impact = ray.origin + ray.direction * t0;

	hit.t = t0;
	hit.origin = impact;
	hit.normal = (impact - sphere.origin) / sphere.radius;

	return hit;
}
