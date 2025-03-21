#version 460 core

#include "core/transform.glsl"

// Forked from https://www.shadertoy.com/view/4ljcz1

// -------------------------------------------------------
// Defines

#define NOISE    iq_noise

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (location = 1) uniform float uniform_dustIntensity;

// --------------------------------------------------------
// Output

out vec4 outColor;

// --------------------------------------------------------
// Shader

//// iq: https://www.shadertoy.com/view/4sfGzS
float iq_hash(vec3 p)
{
    p  = fract( p*0.3183099+.1 );
	p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}
float iq_noise( in vec3 x )
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
	
    return mix(mix(mix( iq_hash(i+vec3(0,0,0)), 
                        iq_hash(i+vec3(1,0,0)),f.x),
                   mix( iq_hash(i+vec3(0,1,0)), 
                        iq_hash(i+vec3(1,1,0)),f.x),f.y),
               mix(mix( iq_hash(i+vec3(0,0,1)), 
                        iq_hash(i+vec3(1,0,1)),f.x),
                   mix( iq_hash(i+vec3(0,1,1)), 
                        iq_hash(i+vec3(1,1,1)),f.x),f.y),f.z);
}
///////////////////

float hash(float n) {
    return fract(sin(n)*758.5453);
}

float configurablenoise(vec3 x, float c1, float c2) {
	vec3 p = floor(x);
	vec3 f = fract(x);
	f      = f*f*(3.0-2.0*f);

	float h2 = c1;
	float h1 = c2;
	#define h3 (h2 + h1)

	float n = p.x*h3 + p.y*h1+ h2*p.z;
	return mix(mix(	mix( hash(n+0.0), hash(n+1.0),f.x),
			mix( hash(n+h1), hash(n+h1+1.0),f.x),f.y),
		   mix(	mix( hash(n+h2), hash(n+h2+1.0),f.x),
			mix( hash(n+h3), hash(n+h3+1.0),f.x),f.y),f.z);
}

float supernoise3dX(vec3 p) {
	float a = configurablenoise(p, 883.0, 971.0);
	float b = configurablenoise(p * 2.0, 113.0, 157.0);
	return (a * b);
}

float fbmHI2d(vec2 p, float dx) {
    p *= 1.2;
	float a = 0.0;
    float w = 1.0;
    float wc = 0.0;
	for(int i=0;i<5;i++){
		a += clamp(2.0 * abs(0.5 - (supernoise3dX(vec3(p, 1.0)))) * w, 0.0, 1.0);
		wc += w;
        w *= 0.5;
		p = p * dx;
	}
	return a / wc;
}

float fbmHI3d(vec3 p, float dx) {
   // p *= 0.1;
    p *= 1.2;
	//p += getWind(p * 0.2) * 6.0;
	float a = 0.0;
    float w = 1.0;
    float wc = 0.0;
	for(int i=0;i<5;i++){
        //p += noise(vec3(a));
		a += clamp(2.0 * abs(0.5 - (NOISE(p))) * w, 0.0, 1.0);
		wc += w;
        w *= 0.5;
		p = p * dx;
	}
	return a / wc;// + noise(p * 100.0) * 11;
}

float dustFbm(vec3 p, float dx) {
   // p *= 0.1;
    p *= 1.2;
	//p += getWind(p * 0.2) * 6.0;
	float a = 0.0;
    float w = 1.0;
    float wc = 0.0;
	for(int i=0;i<5;i++){
        //p += noise(vec3(a));
		a += clamp(2.0 * abs(0.5 - (supernoise3dX(p))) * w, 0.0, 1.0);
		wc += w;
        w *= 0.5;
		p = p * dx;
	}
	return a / wc;// + noise(p * 100.0) * 11;
}

float starsInner(vec3 seed, float intensity) {
	float edge0 = 1.0 - intensity * 0.9;
	float edge1 = (1.0 - intensity * 0.9) + 0.1;
	float x = NOISE(seed * 3700.0) * (0.5 + 0.5 * NOISE(seed * 2.0));
	//float x = NOISE(seed * 3700.0);
	return smoothstep(edge0, edge1, x);
}

// This is a complete mess :(
vec3 traceScene(vec3 dir) {
	const float GRAY_LEVEL = 0.27;

	float intensityred   = NOISE(dir * 352.062) * 0.7;
	float intensitywhite = NOISE(dir * 406.211) * 0.7;
	float intensityblue  = NOISE(dir * 703.263) * 0.7;

	intensityred   = smoothstep(GRAY_LEVEL, 1.0, intensityred);
	intensitywhite = smoothstep(GRAY_LEVEL, 1.0, intensitywhite);
	intensityblue  = smoothstep(GRAY_LEVEL, 1.0, intensityblue);
	
	const float baseX = 0.0;
	const float baseZ = 0.2;
	float zFactor = 0.7 * abs(baseZ - dir.y);
	float xFactor = 0.2 * max(0.0, baseX - dir.x);

	float galaxydust = smoothstep(0.1, 0.2, (1.0 / (23.0 * zFactor)) * dustFbm(dir * 62.0, 3.0) * xFactor);
	float galaxydust2 = smoothstep(0.1, 0.43, (1.0 / (40.0 * zFactor)) * dustFbm(dir * 77.0, 3.0) * xFactor);

	float redlights = starsInner(dir * 25.126, intensityred);
	float whitelights = starsInner(dir * 30.0, intensitywhite);
	float bluelights = starsInner(dir * 50.0, intensityblue);

	vec3 starscolor = vec3(1.0, 0.8, 0.5) * redlights
		+ vec3(1.0) * whitelights
		+ vec3(0.6, 0.7, 1.0) * bluelights;

	const vec3 purple = vec3(0.5, 0.0, 0.9);
	const vec3 white = vec3(1.0, 1.0, 1.0);

	vec3 dustinner = uniform_dustIntensity * purple;//vec3(0.9, 0.8, 0.8);
	vec3 dustouter = uniform_dustIntensity * white;
	vec3 innermix = mix(dustinner, starscolor, 1.0 - galaxydust);
	vec3 allmix = mix(dustouter, innermix, 1.0 - galaxydust2);

	return allmix;
	//return starscolor;
}

void main() {
	vec2 uv = fs_in.screenUV;
	vec3 dir = EquirectangularToCube(uv);

// Noise test
#if 0
	vec3 V = OctahedralConcentricMapping(uv);
	//float x1 = hash(fbmHI3d(V, 4.0));
	//float x1 = hash(asin(uv.y) * 634.232);
	float x1 = iq_noise(dir * 552.253);
	outColor = vec4(x1, x1, x1, 1.0);
#elif 0
	vec3 v = dir.xyz;
	//v.z *= sin(dir.z);
	float x1 = iq_hash(v);
	outColor = vec4(x1, x1, x1, 1.0);
#else
    outColor = vec4(traceScene(dir), 1.0);
#endif
}
