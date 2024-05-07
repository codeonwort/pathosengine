//? #version 460 core
// transform.glsl

#include "core/common.glsl"

// Maps a 3D unit direction vector to a 2D uv coordinate.
// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
vec2 CubeToEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // inverse atan
    uv += 0.5;
    return uv;
}

// Maps a 2D uv coordinate to a 3D unit direction vector.
vec3 EquirectangularToCube(vec2 uv) {
	uv -= 0.5;
	uv *= vec2(2.0 * PI, PI);

	// x = cos(u) * cos(v)
	// y = sin(u) * cos(v)
	// z = sin(v)
	return vec3(cos(uv.y) * cos(uv.x), cos(uv.y) * sin(uv.x), sin(uv.y));
}

// Ray Tracing Gems
//   Chapter 16. "Sample Transformations Zoo"
//     16.5.4.2. "Octahedral Concentric (Uniform) Map"
// Unit square to unit sphere mapping
vec3 OctahedralConcentricMapping(vec2 uv) {
	uv = 2.0 * uv - 1.0;
	float d = 1 - (abs(uv.x) + abs(uv.y));
	float r = 1 - abs(d);

	float phi = (r == 0.0) ? 0.0 : (PI / 4.0) * ((abs(uv.x) - abs(uv.y)) / r + 1.0);
	float f = r * sqrt(2.0 - r * r);
	float x = f * sign(uv.x) * cos(phi);
	float y = f * sign(uv.y) * sin(phi);
	float z = sign(d) * (1.0 - r * r);

	return vec3(x, y, z);
}
