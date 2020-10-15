#version 450 core

// Forked from https://www.shadertoy.com/view/4ljcz1

#define PI 3.141592

//#define NOISE supernoise3dX
#define NOISE iq_noise

in VS_OUT {
	vec2 screenUV;
} fs_in;

out vec4 out0;

vec2 CubeToEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // inverse atan
    uv += 0.5;
    return uv;
}

vec3 EquirectangularToCube(vec2 uv) {
	uv -= 0.5;
	uv *= vec2(2.0 * PI, PI);

	return vec3(cos(uv.y) * cos(uv.x), cos(uv.y) * sin(uv.x), sin(uv.y));
}

//// iq ////
// https://www.shadertoy.com/view/4sfGzS
float iq_hash(vec3 p)  // replace this by something better
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

float stars(vec3 seed, float intensity) {
	float edge0 = 1.0 - intensity * 0.9;
	float edge1 = (1.0 - intensity * 0.9) + 0.1;
	float x = NOISE(seed * 3700.0) * (0.5 + 0.5 * NOISE(seed * 2.0));
	//float x = NOISE(seed * 3700.0);
	return smoothstep(edge0, edge1, x);
}

// This is a complete mess :(
vec3 stars(vec3 dir) {
	//float intensityred = (1.0 / (1.0 + 30.0 * abs(uv.y))) * fbmHI2d(uv * 30.0, 3.0) * (1.0 - abs(uv.x ));	
	//float intensitywhite = (1.0 / (1.0 + 20.0 * abs(uv.y))) * fbmHI2d(uv * 30.0 + 120.0, 3.0) * (1.0 - abs(uv.x ));	
	//float intensityblue = (1.0 / (1.0 + 20.0 * abs(uv.y))) * fbmHI2d(uv * 30.0 + 220.0, 3.0) * (1.0 - abs(uv.x ));	
	//intensityred = 1.0 - pow(1.0 - intensityred, 3.0) * 0.73;
	//intensitywhite = 1.0 - pow(1.0 - intensitywhite, 3.0) * 0.73;
	//intensityblue = 1.0 - pow(1.0 - intensityblue, 3.0) * 0.73;
	//float redlights = stars(uv, intensityred );
	//float whitelights = stars(uv, intensitywhite );
	//float bluelights = stars(uv, intensityblue );
	//vec3 starscolor = vec3(1.0, 0.8, 0.5) * redlights + vec3(1.0) * whitelights + vec3(0.6, 0.7, 1.0) * bluelights;
	//vec3 dustinner = vec3(0.9, 0.8, 0.8);
	//vec3 dustouter = vec3(0.2, 0.1, 0.0);
	//vec3 innermix = mix(dustinner, starscolor, 1.0 - galaxydust);
	//vec3 allmix = mix(dustouter, innermix, 1.0 - galaxydust2);
	//vec3 bloom = 1.6 * dustinner * (1.0 / (1.0 + 30.0 * abs(uv.y))) * fbmHI2d(uv * 3.0, 3.0) * (1.0 - abs(uv.x ));	
	//return allmix + bloom;

	const float GRAY_LEVEL = 0.32;

	float intensityred = fbmHI3d(dir * 102.062, 1.6);
	float intensitywhite = fbmHI3d(dir * 106.211, 1.17);
	float intensityblue = fbmHI3d(dir * 103.263, 1.83);

	intensityred = smoothstep(GRAY_LEVEL, 1.0, intensityred);
	intensitywhite = smoothstep(GRAY_LEVEL, 1.0, intensitywhite);
	intensityblue = smoothstep(GRAY_LEVEL, 1.0, intensityblue);
	
	const float baseX = 0.0;
	const float baseZ = 0.2;
	float zFactor = 0.7 * abs(baseZ - dir.z);
	float xFactor = 0.2 * max(0.0, baseX - dir.x);

	//float galaxydust = smoothstep(0.1, 0.5, (1.0 / (1.0 + 20.0 * abs(uv.y))) * fbmHI2d(uv * 20.0 + 220.0, 3.0) * (1.0 - abs(uv.x)));
	float galaxydust = smoothstep(0.1, 0.2, (1.0 / (23.0 * zFactor)) * dustFbm(dir * 62.0, 3.0) * xFactor);
	float galaxydust2 = smoothstep(0.1, 0.43, (1.0 / (40.0 * zFactor)) * dustFbm(dir * 77.0, 3.0) * xFactor);

	//galaxydust = smoothstep(0.1, 0.5, (1.0 / (1.0 + 30.0 * zFactor)) * fbmHI3d(dir * 30.0, 3.0));
	//galaxydust2 = smoothstep(0.1, 0.5, (1.0 / (1.0 + 20.0 * zFactor)) * fbmHI3d(dir * 20.0, 3.0));

	float redlights = stars(dir * 100.0, intensityred);
	float whitelights = stars(dir * 100.0, intensitywhite);
	float bluelights = stars(dir * 100.0, intensityblue);

	vec3 starscolor = vec3(1.0, 0.8, 0.5) * redlights
		+ vec3(1.0) * whitelights
		+ vec3(0.6, 0.7, 1.0) * bluelights;

	const vec3 purple = vec3(0.5, 0.0, 0.9);
	const vec3 white = vec3(1.0, 1.0, 1.0);

	vec3 dustinner = purple;//vec3(0.9, 0.8, 0.8);
	vec3 dustouter = white;
	vec3 innermix = mix(dustinner, starscolor, 1.0 - galaxydust);
	vec3 allmix = mix(dustouter, innermix, 1.0 - galaxydust2);
	vec3 bloom = 1.6 * dustinner * (1.0 / (1.0 + 15.0 * zFactor)) * dustFbm(dir * 3.0, 3.0) * xFactor;

	return allmix;
	//return allmix + bloom;
	//return starscolor;
}

void main() {
	vec2 uv = fs_in.screenUV;
	//uv.x += 0.01 * (0.5 - configurablenoise(260.0 * vec3(uv, 2.0), 126.3, 62.42));
	//uv.y += 0.01 * (0.5 - configurablenoise(120.0 * vec3(uv, 1.0), 626.1, 33.27));

	vec3 dir = EquirectangularToCube(uv);

	//out0 = vec4(vec3(fbmHI3d(dir, 3.0)), 1.0);
    out0 = vec4(stars(dir), 1.0);
}
