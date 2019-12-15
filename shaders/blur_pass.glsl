#version 430 core

layout (binding = 0) uniform sampler2D src;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

layout (location = 0) uniform bool horizontal;

out vec4 out_color;

void main() {
	ivec2 origin = ivec2(gl_FragCoord.xy);
	vec4 fetch = texelFetch(src, origin, 0);
	vec3 result = fetch.rgb * weight[0];
	if(horizontal) {
		for(int i = 1; i < 5; ++i) {
			result += texelFetch(src, origin + ivec2(1, 0), 0).rgb * weight[i];
			result += texelFetch(src, origin - ivec2(1, 0), 0).rgb * weight[i];
		}
	} else {
		for(int i = 1; i < 5; ++i) {
			result += texelFetch(src, origin + ivec2(0, 1), 0).rgb * weight[i];
			result += texelFetch(src, origin - ivec2(0, 1), 0).rgb * weight[i];
		}
	}

	// w component is for depth-of-field. continue to tone_mapping.glsl
	out_color = vec4(result, fetch.a);
}
