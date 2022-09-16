#version 460 core

layout (location = 0) uniform mat4 viewProj;

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

out VS_OUT {
	vec3 r;
} vs_out;

void main() {
	vs_out.r = position;
	gl_Position = (viewProj * vec4(position, 1)).xyww;
#if REVERSE_Z
	gl_Position.z = 0.0;
#endif
}

#endif // VERTEX_SHADER

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

layout (binding = 0) uniform sampler2D texSky;

in VS_OUT {
	vec3 r;
} fs_in;

layout (location = 0) out vec4 outSceneColor;

vec2 CubeToEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // inverse atan
    uv += 0.5;
    return uv;
}

void main() {
#if 0
	const float PI = 3.14159265359;
	vec3 r0 = normalize(fs_in.r);
	vec3 r = vec3(r0.x, r0.z, -r0.y);
	vec2 tc;
	tc.x = (atan(r.y, r.x) + PI) / PI * 0.5;
	tc.y = acos(r.z) / PI;
#else
	vec2 tc = CubeToEquirectangular(normalize(fs_in.r));
#endif

	vec3 sky = texture(texSky, tc).xyz;
	outSceneColor = vec4(sky, 1.0);
}

#endif // FRAGMENT_SHADER
