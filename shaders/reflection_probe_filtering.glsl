#version 460 core

#include "reflection_probe_coeffs_const_32.glsl"

// "Fast Filtering of Reflection Probes", Josiah Manson and Peter-Pike Sloan.

// Note for integration
// - asd
//


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

#define GROUP_SIZE 64

layout (local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0) uniform samplerCube tex_in;
layout (binding = 0, rgba16f) writeonly uniform imageCube tex_out0;
layout (binding = 1, rgba16f) writeonly uniform imageCube tex_out1;
layout (binding = 2, rgba16f) writeonly uniform imageCube tex_out2;
layout (binding = 3, rgba16f) writeonly uniform imageCube tex_out3;
layout (binding = 4, rgba16f) writeonly uniform imageCube tex_out4;
layout (binding = 5, rgba16f) writeonly uniform imageCube tex_out5;
layout (binding = 6, rgba16f) writeonly uniform imageCube tex_out6;

#define NUM_TAPS 32
#define BASE_RESOLUTION 128

void get_dir(out vec3 dir, in vec2 uv, in int face) {
	switch (face) {
		case 0: dir = vec3(1, uv[1], -uv[0]); break;
		case 1: dir = vec3(-1, uv[1], uv[0]); break;
		case 2: dir = vec3(uv[0], 1, -uv[1]); break;
		case 3: dir = vec3(uv[0], -1, uv[1]); break;
		case 4: dir = vec3(uv[0], uv[1], 1); break;
		default: dir = vec3(-uv[0], uv[1], -1); break;
	}
}

void main() {
	uvec3 id = gl_GlobalInvocationID.xyz;

	// INPUT: 
	// id.x = the linear address of the texel (ignoring face)
	// id.y = the face
	// -> use to index output texture
	// id.x = texel x
	// id.y = texel y
	// id.z = face

	// determine which texel this is
	int level = 0;
	if (id.x < (128 * 128)) {
		level = 0;
	} else if (id.x < (128 * 128 + 64 * 64)) {
		level = 1;
		id.x -= (128 * 128);
	} else if (id.x < (128 * 128 + 64 * 64 + 32 * 32)) {
		level = 2;
		id.x -= (128 * 128 + 64 * 64);
	} else if (id.x < (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16)) {
		level = 3;
		id.x -= (128 * 128 + 64 * 64 + 32 * 32);
	} else if (id.x < (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8)) {
		level = 4;
		id.x -= (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16);
	} else if (id.x < (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4)) {
		level = 5;
		id.x -= (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8);
	} else if (id.x < (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 + 2 * 2)) {
		level = 6;
		id.x -= (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4);
	} else {
		return;
	}

	// #todo: Sample code is missing the implementation for this function.
	// Somehow it forces 'level' to reside in a scalar register? Or is it a wave operation? I can't know.
	//SetSGPR(level);

	// determine dir / pos for the texel
	vec3 dir, adir, frameZ;
	{
		id.z = id.y;
		int res = BASE_RESOLUTION >> level;
		id.y = id.x / res;
		id.x -= id.y * res;

		vec2 uv;
		uv.x = (float(id.x) * 2.0f + 1.0f) / float(res) - 1.0f;
		uv.y = -(float(id.y) * 2.0f + 1.0f) / float(res) + 1.0f;

		get_dir(dir, uv, int(id.z));
		frameZ = normalize(dir);

		adir = abs(dir);
		//adir[0] = abs(dir[0]);
		//adir[1] = abs(dir[1]);
		//adir[2] = abs(dir[2]);
	}

	// GGX gather colors
	vec4 color = vec4(0);
	for (int axis = 0; axis < 3; axis++) {
		const int otherAxis0 = 1 - (axis & 1) - (axis >> 1);
		const int otherAxis1 = 2 - (axis >> 1);

		float frameweight = (max(adir[otherAxis0], adir[otherAxis1]) - .75f) / .25f;
		if (frameweight > 0) {
			// determine frame
#if 0
			vec3 UpVector = 0;
			UpVector[axis] = 1;
#else
			vec3 UpVector;
			switch (axis) {
			case 0:
				UpVector = vec3(1, 0, 0);
				break;
			case 1:
				UpVector = vec3(0, 1, 0);
				break;
			default:
				UpVector = vec3(0, 0, 1);
				break;
			}
#endif
			vec3 frameX = normalize(cross(UpVector, frameZ));
			vec3 frameY = cross(frameZ, frameX);

			// calculate parametrization for polynomial
			float Nx = dir[otherAxis0];
			float Ny = dir[otherAxis1];
			float Nz = adir[axis];

			float NmaxXY = max(abs(Ny), abs(Nx));
			Nx /= NmaxXY;
			Ny /= NmaxXY;

			float theta;
			if (Ny < Nx) {
				if (Ny <= -.999)
					theta = Nx;
				else
					theta = Ny;
			} else {
				if (Ny >= .999)
					theta = -Nx;
				else
					theta = -Ny;
			}

			float phi;
			if (Nz <= -.999)
				phi = -NmaxXY;
			else if (Nz >= .999)
				phi = NmaxXY;
			else
				phi = Nz;

			float theta2 = theta * theta;
			float phi2 = phi * phi;

			// sample
			for (int iSuperTap = 0; iSuperTap < NUM_TAPS / 4; iSuperTap++) {
				const int index = (NUM_TAPS / 4) * axis + iSuperTap;
				vec4 coeffsDir0[3];
				vec4 coeffsDir1[3];
				vec4 coeffsDir2[3];
				vec4 coeffsLevel[3];
				vec4 coeffsWeight[3];

				for (int iCoeff = 0; iCoeff < 3; iCoeff++) {
					coeffsDir0[iCoeff]   = vec4(coeffs[level][0][iCoeff][index]);
					coeffsDir1[iCoeff]   = vec4(coeffs[level][1][iCoeff][index]);
					coeffsDir2[iCoeff]   = vec4(coeffs[level][2][iCoeff][index]);
					coeffsLevel[iCoeff]  = vec4(coeffs[level][3][iCoeff][index]);
					coeffsWeight[iCoeff] = vec4(coeffs[level][4][iCoeff][index]);
				}

				for (int iSubTap = 0; iSubTap < 4; iSubTap++) {
					// determine sample attributes (dir, weight, level)
					vec3 sample_dir
						= frameX * (coeffsDir0[0][iSubTap] + coeffsDir0[1][iSubTap] * theta2 + coeffsDir0[2][iSubTap] * phi2)
						+ frameY * (coeffsDir1[0][iSubTap] + coeffsDir1[1][iSubTap] * theta2 + coeffsDir1[2][iSubTap] * phi2)
						+ frameZ * (coeffsDir2[0][iSubTap] + coeffsDir2[1][iSubTap] * theta2 + coeffsDir2[2][iSubTap] * phi2);

					float sample_level = coeffsLevel[0][iSubTap] + coeffsLevel[1][iSubTap] * theta2 + coeffsLevel[2][iSubTap] * phi2;

					float sample_weight = coeffsWeight[0][iSubTap] + coeffsWeight[1][iSubTap] * theta2 + coeffsWeight[2][iSubTap] * phi2;
					sample_weight *= frameweight;

					// adjust for jacobian
					sample_dir /= max(abs(sample_dir[0]), max(abs(sample_dir[1]), abs(sample_dir[2])));
					sample_level += 0.75f * log2(dot(sample_dir, sample_dir));

					// sample cubemap
					color.xyz += textureLod(tex_in, sample_dir, sample_level).xyz * sample_weight;
					color.w += sample_weight;
				}
			}
		}
	}
	color /= color.w;

	// write color
	color.x = max(0, color.x);
	color.y = max(0, color.y);
	color.z = max(0, color.z);
	color.w = 1;

	switch (level) {
		case 0:
			imageStore(tex_out0, ivec3(id), color);
			break;
		case 1:
			imageStore(tex_out1, ivec3(id), color);
			break;
		case 2:
			imageStore(tex_out2, ivec3(id), color);
			break;
		case 3:
			imageStore(tex_out3, ivec3(id), color);
			break;
		case 4:
			imageStore(tex_out4, ivec3(id), color);
			break;
		case 5:
			imageStore(tex_out5, ivec3(id), color);
			break;
		default:
			imageStore(tex_out6, ivec3(id), color);
			break;
	}
}
