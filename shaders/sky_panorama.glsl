#version 460 core

// --------------------------------------------------------
// Input

layout (std140, binding = 1) uniform UBO_PanoramaSky {
	mat4 viewProj;
	float intensity;
} ubo;

// --------------------------------------------------------
// Vertex Shader

#if VERTEX_SHADER

layout (location = 0) in vec3 inPosition;

out VS_OUT {
	vec3 r;
} vs_out;

void main() {
	vs_out.r = inPosition;
	gl_Position = (ubo.viewProj * vec4(inPosition, 1)).xyww;
#if REVERSE_Z
	gl_Position.z = 0.0;
#endif
}

#endif // VERTEX_SHADER

// --------------------------------------------------------
// Fragment Shader

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

	vec3 sky = ubo.intensity * texture(texSky, tc).xyz;
	outSceneColor = vec4(sky, 1.0);
}

#endif // FRAGMENT_SHADER
