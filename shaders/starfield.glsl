#version 450 core

// Forked from https://www.shadertoy.com/view/4ljcz1

#define PI 3.141592

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
    float u = uv.x * PI * 2.0;
    float v = (uv.y - 0.5) * PI;
    return vec3(cos(v) * cos(u), cos(v) * sin(u), sin(v));
}

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

	float n = p.x + p.y*h1+ h2*p.z;
	return mix(mix(	mix( hash(n+0.0), hash(n+1.0),f.x),
			mix( hash(n+h1), hash(n+h1+1.0),f.x),f.y),
		   mix(	mix( hash(n+h2), hash(n+h2+1.0),f.x),
			mix( hash(n+h3), hash(n+h3+1.0),f.x),f.y),f.z);
}

float supernoise3dX(vec3 p) {
	float a = configurablenoise(p, 883.0, 971.0);
	float b = configurablenoise(p + 0.5, 113.0, 157.0);
	return (a * b);
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
		a += clamp(2.0 * abs(0.5 - (supernoise3dX(p))) * w, 0.0, 1.0);
		wc += w;
        w *= 0.5;
		p = p * dx;
	}
	return a / wc;// + noise(p * 100.0) * 11;
}

float stars(vec3 seed, float intensity) {
	float edge0 = 1.0 - intensity * 0.9;
	float edge1 = (1.0 - intensity *0.9) + 0.1;
	//float x = supernoise3dX(seed * 500.0) * (0.8 + 0.2 * supernoise3dX(seed * 40.0));
	float x = supernoise3dX(seed * 3700.0);
	return smoothstep(edge0, edge1, x);
}
vec3 stars(vec3 dir) {
	//float intensityred = (1.0 / (1.0 + 30.0 * abs(uv.y))) * fbmHI2d(uv * 30.0, 3.0) * (1.0 - abs(uv.x ));	
	//float intensitywhite = (1.0 / (1.0 + 20.0 * abs(uv.y))) * fbmHI2d(uv * 30.0 + 120.0, 3.0) * (1.0 - abs(uv.x ));	
	//float intensityblue = (1.0 / (1.0 + 20.0 * abs(uv.y))) * fbmHI2d(uv * 30.0 + 220.0, 3.0) * (1.0 - abs(uv.x ));	
	//float galaxydust = smoothstep(0.1, 0.5, (1.0 / (1.0 + 20.0 * abs(uv.y))) * fbmHI2d(uv * 20.0 + 220.0, 3.0) * (1.0 - abs(uv.x )));	
	//float galaxydust2 = smoothstep(0.1, 0.5, (1.0 / (1.0 + 20.0 * abs(uv.y))) * fbmHI2d(uv * 50.0 + 220.0, 3.0) * (1.0 - abs(uv.x )));	
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

	// TODO: perturbate dir itself with noise. It's making a donut pattern for now...
	//dir = vec3(fbmHI3d(dir.xyz, 10.0), fbmHI3d(dir.yzx, 30.0), fbmHI3d(dir.zxy, 20.0));

	float intensityred = fbmHI3d(dir * 12.062, 13.6);
	float intensitywhite = fbmHI3d(dir * 16.211, 14.17);
	float intensityblue = fbmHI3d(dir * 13.263, 12.83);

	float redlights = stars(dir, intensityred);
	float whitelights = stars(dir, intensitywhite);
	float bluelights = stars(dir, intensityblue);

	vec3 starscolor = vec3(1.0, 0.8, 0.5) * redlights
		+ vec3(1.0) * whitelights
		+ vec3(0.6, 0.7, 1.0) * bluelights;

	return starscolor;

	float fbm = fbmHI3d(dir, 3.0);
	return vec3(fbm);
}

void main() {
	vec3 dir = EquirectangularToCube(fs_in.screenUV);

    out0 = vec4(stars(dir), 1.0);

	//out0.rgb = vec3(fbmHI3d(dir * 30.0, 3.0));
}
