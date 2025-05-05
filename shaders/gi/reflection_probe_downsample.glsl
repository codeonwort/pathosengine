#version 460 core

// "Fast Filtering of Reflection Probes", Josiah Manson and Peter-Pike Sloan.

// Note for integration
// - Prepare a cubemap whose size is 128 and mip count is 8. Render the scene to all faces of mip 0 of the cubemap.
// - Invoke this shader 7 times.
//
// Pseudo code:
//   int size = 128;
//   glTextureParameteri(cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   for (int i = 0; i < 7; ++i) {
//     size = size / 2;
//     glTextureParameteri(cubemap, GL_TEXTURE_BASE_LEVEL, i); // Let textureLod() samples mip i
//     glBindTextureUnit(0, cubemap); // Bind tex_hi_res
//     glBindImageTexture(0, cubemap, i + 1, GL_TRUE, 6, GL_WRITE_ONLY, GL_RGBA16F); // Bind tex_lo_res
//     glDispatchCompute((size + 7) / 8, (size + 7) / 8, 6);
//   }

// Copyright 2016 Activision Publishing, Inc.
// 
// Permission is hereby granted, free of charge, to any person obtaining 
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software 
// is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (binding = 0) uniform samplerCube tex_hi_res;
layout (binding = 0, rgba16f) writeonly uniform imageCube tex_lo_res;

void get_dir_0(out vec3 dir, in float u, in float v) { dir = vec3(1, v, -u); }
void get_dir_1(out vec3 dir, in float u, in float v) { dir = vec3(-1, v, u); }
void get_dir_2(out vec3 dir, in float u, in float v) { dir = vec3(u, 1, -v); }
void get_dir_3(out vec3 dir, in float u, in float v) { dir = vec3(u, -1, v); }
void get_dir_4(out vec3 dir, in float u, in float v) { dir = vec3(u, v, 1); }
void get_dir_5(out vec3 dir, in float u, in float v) { dir = vec3(-u, v, -1); }

void get_dir(out vec3 dir, in float u, in float v, in int face) {
	switch (face) {
		case 0: get_dir_0(dir, u, v); break;
		case 1: get_dir_1(dir, u, v); break;
		case 2: get_dir_2(dir, u, v); break;
		case 3: get_dir_3(dir, u, v); break;
		case 4: get_dir_4(dir, u, v); break;
		default: get_dir_5(dir, u, v); break;
	}
}

float calcWeight(float u, float v) {
	float val = u * u + v * v + 1;
	return val * sqrt(val);
}

void main() {
	uvec3 id = gl_GlobalInvocationID.xyz;

	uint res_lo = uint(imageSize(tex_lo_res).x);

	if (id.x < res_lo && id.y < res_lo) {
		float inv_res_lo = 1.0 / float(res_lo);

		float u0 = (float(id.x) * 2.0f + 1.0f - .75f) * inv_res_lo - 1.0f;
		float u1 = (float(id.x) * 2.0f + 1.0f + .75f) * inv_res_lo - 1.0f;

		float v0 = (float(id.y) * 2.0f + 1.0f - .75f) * -inv_res_lo + 1.0f;
		float v1 = (float(id.y) * 2.0f + 1.0f + .75f) * -inv_res_lo + 1.0f;

		float weights[4];
		weights[0] = calcWeight(u0, v0);
		weights[1] = calcWeight(u1, v0);
		weights[2] = calcWeight(u0, v1);
		weights[3] = calcWeight(u1, v1);

		const float wsum = 0.5f / (weights[0] + weights[1] + weights[2] + weights[3]);
		#pragma unroll 4
		for (int i = 0; i < 4; i++) {
			weights[i] = weights[i] * wsum + .125f;
		}

#if 1
		vec3 dir;
		vec4 color;
		switch (id.z) {
		case 0:
			get_dir_0(dir, u0, v0);
			color = textureLod(tex_hi_res, dir, 0) * weights[0];

			get_dir_0(dir, u1, v0);
			color += textureLod(tex_hi_res, dir, 0) * weights[1];

			get_dir_0(dir, u0, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[2];

			get_dir_0(dir, u1, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[3];
			break;
		case 1:
			get_dir_1(dir, u0, v0);
			color = textureLod(tex_hi_res, dir, 0) * weights[0];

			get_dir_1(dir, u1, v0);
			color += textureLod(tex_hi_res, dir, 0) * weights[1];

			get_dir_1(dir, u0, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[2];

			get_dir_1(dir, u1, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[3];
			break;
		case 2:
			get_dir_2(dir, u0, v0);
			color = textureLod(tex_hi_res, dir, 0) * weights[0];

			get_dir_2(dir, u1, v0);
			color += textureLod(tex_hi_res, dir, 0) * weights[1];

			get_dir_2(dir, u0, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[2];

			get_dir_2(dir, u1, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[3];
			break;
		case 3:
			get_dir_3(dir, u0, v0);
			color = textureLod(tex_hi_res, dir, 0) * weights[0];

			get_dir_3(dir, u1, v0);
			color += textureLod(tex_hi_res, dir, 0) * weights[1];

			get_dir_3(dir, u0, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[2];

			get_dir_3(dir, u1, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[3];
			break;
		case 4:
			get_dir_4(dir, u0, v0);
			color = textureLod(tex_hi_res, dir, 0) * weights[0];

			get_dir_4(dir, u1, v0);
			color += textureLod(tex_hi_res, dir, 0) * weights[1];

			get_dir_4(dir, u0, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[2];

			get_dir_4(dir, u1, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[3];
			break;
		default:
			get_dir_5(dir, u0, v0);
			color = textureLod(tex_hi_res, dir, 0) * weights[0];

			get_dir_5(dir, u1, v0);
			color += textureLod(tex_hi_res, dir, 0) * weights[1];

			get_dir_5(dir, u0, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[2];

			get_dir_5(dir, u1, v1);
			color += textureLod(tex_hi_res, dir, 0) * weights[3];
			break;
		}
#else
		vec3 dir;
		get_dir(dir, u0, v0, id.z);
		vec4 color = textureLod(tex_hi_res, dir, 0) * weights[0];

		get_dir(dir, u1, v0, id.z);
		color += textureLod(tex_hi_res, dir, 0) * weights[1];

		get_dir(dir, u0, v1, id.z);
		color += textureLod(tex_hi_res, dir, 0) * weights[2];

		get_dir(dir, u1, v1, id.z);
		color += textureLod(tex_hi_res, dir, 0) * weights[3];
#endif

		imageStore(tex_lo_res, ivec3(id), color);
	}
}
