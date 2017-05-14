#include "pathos/engine.h"
#include "deferredpass_unpack.h"
#include <algorithm>
using std::min;

#if defined(_DEBUG)
#include <iostream>
#endif

namespace pathos {

	MeshDeferredRenderPass_Unpack::MeshDeferredRenderPass_Unpack(GLuint gbuffer_tex0, GLuint gbuffer_tex1, unsigned int width, unsigned int height)
		: gbuffer_tex0(gbuffer_tex0), gbuffer_tex1(gbuffer_tex1)
	{
		quad = new PlaneGeometry(2.0f, 2.0f);
		createProgram();
		createResource_HDR(width, height);
		godRay = new GodRay(width, height);
	}

	MeshDeferredRenderPass_Unpack::~MeshDeferredRenderPass_Unpack() {
		glDeleteProgram(program);
		glDeleteProgram(program_hdr);
		glDeleteProgram(program_tone_mapping);
		glDeleteFramebuffers(1, &fbo_hdr);
		glDeleteTextures(NUM_HDR_ATTACHMENTS, fbo_hdr_attachment);
		glDeleteProgram(program_blur);
		glDeleteFramebuffers(1, &fbo_blur);
		glDeleteTextures(1, &fbo_blur_attachment);
		quad->dispose();
		delete godRay;
	}

	void MeshDeferredRenderPass_Unpack::createProgram() {
		createProgram_LDR();
		createProgram_HDR();
	}

	void MeshDeferredRenderPass_Unpack::createProgram_LDR() {
		string vshader = R"(#version 430 core

layout (location = 0) in vec3 position;

void main() {
	gl_Position = vec4(position, 1.0);
}

)";
		// length of dirLightDirs and dirLightColors should match with MeshDeferredRenderPass_Unpack::MAX_DIRECTIONAL_LIGHTS
		string fshader = R"(#version 430 core

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;

uniform vec3 eye;

uniform uint numDirLights;
uniform vec3 dirLightDirs[8];
uniform vec3 dirLightColors[8];

uniform uint numPointLights;
uniform vec3 pointLightPos[16];
uniform vec3 pointLightColors[16];

struct fragment_info {
	vec3 color;
	vec3 normal;
	float specular_power;
	vec3 ws_coords;
	uint material_id;
};

void unpackGBuffer(ivec2 coord, out fragment_info fragment) {
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	vec2 temp = unpackHalf2x16(data0.y);

	fragment.color = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.normal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	fragment.material_id = data0.w;

	fragment.ws_coords = data1.xyz;
	fragment.specular_power = data1.w;
}

vec4 calculateShading(fragment_info fragment) {
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if(fragment.material_id != 0){
		vec3 N = fragment.normal;
		for(uint i = 0; i < numDirLights; ++i) {
			vec3 L = -dirLightDirs[i];
			float cosTheta = max(0.0, dot(N, L));
			vec3 diffuse_color = dirLightColors[i] * fragment.color * cosTheta;
			result += vec4(diffuse_color, 0.0);
		}
		for(uint i = 0; i < numPointLights; ++i) {
			vec3 L = pointLightPos[i] - fragment.ws_coords;
			float dist = length(L);
			float attenuation = 500.0 / (pow(dist, 2.0) + 1.0);
			L = normalize(L);
			vec3 R = reflect(-L, N);
			float cosTheta = max(0.0, dot(N, L));
			vec3 specular_color = pointLightColors[i] * pow(max(0.0, dot(R, -eye)), fragment.specular_power);
			vec3 diffuse_color = pointLightColors[i] * fragment.color * cosTheta;
			result += vec4(attenuation * (diffuse_color + specular_color), 0.0);
		}
	}else discard;
	return result;
}

void main() {
	fragment_info fragment;
	unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);
	out_color = calculateShading(fragment);
}

)";

		program = pathos::createProgram(vshader, fshader);
	}

	void MeshDeferredRenderPass_Unpack::createProgram_HDR() {
		string vshader = R"(#version 430 core

layout (location = 0) in vec3 position;

void main() {
	gl_Position = vec4(position, 1.0);
}

)";
		// length of dirLightDirs and dirLightColors should match with MeshDeferredRenderPass_Unpack::MAX_DIRECTIONAL_LIGHTS
		string fshader = R"(#version 430 core

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright; // bright area only

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;

uniform vec3 eye;

uniform uint numDirLights;
uniform vec3 dirLightDirs[8];
uniform vec3 dirLightColors[8];

uniform uint numPointLights;
uniform vec3 pointLightPos[16];
uniform vec3 pointLightColors[16];

// bloom threshold
const float bloom_min = 0.8;
const float bloom_max = 1.2;

struct fragment_info {
	vec3 color;
	vec3 normal;
	float specular_power;
	vec3 ws_coords;
	uint material_id;
};

void unpackGBuffer(ivec2 coord, out fragment_info fragment) {
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	vec2 temp = unpackHalf2x16(data0.y);

	fragment.color = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.normal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	fragment.material_id = data0.w;

	fragment.ws_coords = data1.xyz;
	fragment.specular_power = data1.w;
}

vec4 calculateShading(fragment_info fragment) {
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if(fragment.material_id != 0){
		vec3 N = fragment.normal;
		for(uint i = 0; i < numDirLights; ++i) {
			vec3 L = -dirLightDirs[i];
			float cosTheta = max(0.0, dot(N, L));
			vec3 diffuse_color = dirLightColors[i] * fragment.color * cosTheta;
			result += vec4(diffuse_color, 0.0);
		}
		for(uint i = 0; i < numPointLights; ++i) {
			vec3 L = pointLightPos[i] - fragment.ws_coords;
			float dist = length(L);
			float attenuation = 500.0 / (pow(dist, 2.0) + 1.0);
			L = normalize(L);
			vec3 R = reflect(-L, N);
			float cosTheta = max(0.0, dot(N, L));
			vec3 specular_color = pointLightColors[i] * pow(max(0.0, dot(R, -eye)), fragment.specular_power);
			vec3 diffuse_color = pointLightColors[i] * fragment.color * cosTheta;
			result += vec4(attenuation * (diffuse_color + specular_color), 0.0);
		}
	} else discard;
	return result;
}

void main() {
	fragment_info fragment;
	unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);
	vec4 color = calculateShading(fragment);

	// output: standard shading
	out_color = color;

	// output: light bloom
	float Y = dot(color.xyz, vec3(0.299, 0.587, 0.144));
	color.xyz = color.xyz * 4.0 * smoothstep(bloom_min, bloom_max, Y);
	out_bright = color;
}

)";

		program_hdr = pathos::createProgram(vshader, fshader);

		// tone mapping
		fshader = R"(
#version 430 core

layout (binding = 0) uniform sampler2D hdr_image;
layout (binding = 1) uniform sampler2D hdr_bloom;
layout (binding = 2) uniform sampler2D god_ray;

uniform float exposure = 1.0; // TODO: set this in application-side

out vec4 color;

void main() {
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec4 c = texelFetch(hdr_image, uv, 0);
	c += texelFetch(hdr_bloom, uv, 0);
	c += texelFetch(god_ray, uv, 0);

	c.rgb = vec3(1.0) - exp(-c.rgb * exposure);
	color = c;
}

)";
		program_tone_mapping = pathos::createProgram(vshader, fshader);

		// blur pass
		fshader = R"(
#version 430 core

layout (binding = 0) uniform sampler2D src;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

uniform bool horizontal;

out vec4 out_color;

void main() {
	ivec2 origin = ivec2(gl_FragCoord.xy);
	vec3 result = texelFetch(src, origin, 0).rgb * weight[0];
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
	out_color = vec4(result, 1.0);
}

)";

		program_blur = pathos::createProgram(vshader, fshader);
	}

	void MeshDeferredRenderPass_Unpack::createResource_HDR(unsigned int width, unsigned int height) {
		glGenFramebuffers(1, &fbo_hdr);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_hdr);

		glGenTextures(NUM_HDR_ATTACHMENTS, fbo_hdr_attachment);

		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[1]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_hdr_attachment[0], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_hdr_attachment[1], 0);

		// check if our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#if defined(_DEBUG)
			std::cerr << "Cannot create a framebuffer for HDR" << std::endl;
#endif
			assert(0);
		}

		// blur resource
		glGenFramebuffers(1, &fbo_blur);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur);

		glGenTextures(1, &fbo_blur_attachment);
		glBindTexture(GL_TEXTURE_2D, fbo_blur_attachment);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_blur_attachment, 0);

		GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, draw_buffers);

		// check if our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#if defined(_DEBUG)
			std::cerr << "Cannot create a framebuffer for HDR" << std::endl;
#endif
			assert(0);
		}

		// restore default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void MeshDeferredRenderPass_Unpack::bindFramebuffer(bool hdr) {
		if (hdr) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_hdr);
			static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			glClearBufferfv(GL_COLOR, 0, zero);
			glClearBufferfv(GL_COLOR, 1, zero);
		} else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDrawBuffer(GL_BACK);
		}
	}

	// This should be called after bindFramebuffer
	void MeshDeferredRenderPass_Unpack::setDrawBuffers(bool both) {
		if (both) {
			GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(NUM_HDR_ATTACHMENTS, draw_buffers);
		} else {
			GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, draw_buffers);
		}
	}

	void MeshDeferredRenderPass_Unpack::render(Scene* scene, Camera* camera) {
		glUseProgram(program);

		// texture binding
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex1);

		// specified in shader code
		//glUniform1i(glGetUniformLocation(program, "gbuf0"), 0);
		//glUniform1i(glGetUniformLocation(program, "gbuf1"), 1);

		// uniform: light
		uploadDirectionalLightUniform(scene, MAX_DIRECTIONAL_LIGHTS);
		uploadPointLightUniform(scene, MAX_POINT_LIGHTS);

		// uniform: camera
		const glm::vec3& eye = camera->getEyeVector();
		glUniform3fv(glGetUniformLocation(program, "eye"), 1, &eye[0]);

		glDisable(GL_DEPTH_TEST);

		quad->activatePositionBuffer(0);
		quad->activateIndexBuffer();
		quad->draw();
		quad->deactivatePositionBuffer(0);
		quad->deactivateIndexBuffer();

		glEnable(GL_DEPTH_TEST);
	}

	void MeshDeferredRenderPass_Unpack::renderHDR(Scene* scene, Camera* camera) {
		// prepare god ray image
		godRay->render(scene, camera);

		// bind
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_hdr);
		glUseProgram(program_hdr);

		// texture binding
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex1);

		// uniform: light
		uploadDirectionalLightUniform(scene, MAX_DIRECTIONAL_LIGHTS);
		uploadPointLightUniform(scene, MAX_POINT_LIGHTS);

		// uniform: camera
		const glm::vec3& eye = camera->getEyeVector();
		glUniform3fv(glGetUniformLocation(program, "eye"), 1, &eye[0]);

		glDisable(GL_DEPTH_TEST);

		// render HDR image
		quad->activatePositionBuffer(0);
		quad->activateIndexBuffer();
		quad->draw();
		//quad->deactivatePositionBuffer(0);
		//quad->deactivateIndexBuffer();

		// blur bright area
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_blur_attachment, 0);
		glUseProgram(program_blur);
		glUniform1i(glGetUniformLocation(program_blur, "horizontal"), GL_TRUE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[1]);
		quad->draw();
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_hdr_attachment[1], 0);
		glUniform1i(glGetUniformLocation(program_blur, "horizontal"), GL_FALSE);
		glBindTexture(GL_TEXTURE_2D, fbo_blur_attachment);
		quad->draw();

		// tone mapping
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(program_tone_mapping);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, godRay->getTexture());

		//quad->activatePositionBuffer(0);
		//quad->activateIndexBuffer();
		quad->draw();
		quad->deactivatePositionBuffer(0);
		quad->deactivateIndexBuffer();

		// unbind
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_DEPTH_TEST);
	}

	void MeshDeferredRenderPass_Unpack::uploadDirectionalLightUniform(Scene* scene, unsigned int maxDirectionalLights) {
		if (program == 0) return;
		unsigned int numDirLights = min(scene->directionalLights.size(), maxDirectionalLights);
		glUniform1ui(glGetUniformLocation(program, "numDirLights"), numDirLights);
		if (numDirLights > 0) {
			glUniform3fv(glGetUniformLocation(program, "dirLightDirs"), numDirLights, scene->getDirectionalLightDirectionBuffer());
			glUniform3fv(glGetUniformLocation(program, "dirLightColors"), numDirLights, scene->getDirectionalLightColorBuffer());
		}
	}
	void MeshDeferredRenderPass_Unpack::uploadPointLightUniform(Scene* scene, unsigned int maxPointLights) {
		if (program == 0) return;
		unsigned int numPointLights = min(scene->pointLights.size(), maxPointLights);
		glUniform1ui(glGetUniformLocation(program, "numPointLights"), numPointLights);
		if (numPointLights) {
			glUniform3fv(glGetUniformLocation(program, "pointLightPos"), numPointLights, scene->getPointLightPositionBuffer());
			glUniform3fv(glGetUniformLocation(program, "pointLightColors"), numPointLights, scene->getPointLightColorBuffer());
		}
	}

}