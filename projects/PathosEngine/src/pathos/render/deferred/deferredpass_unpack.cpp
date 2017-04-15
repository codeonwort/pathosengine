#include "deferredpass_unpack.h"
#include <algorithm>
using std::min;

namespace pathos {

	MeshDeferredRenderPass_Unpack::MeshDeferredRenderPass_Unpack(GLuint gbuffer_tex0, GLuint gbuffer_tex1)
		: gbuffer_tex0(gbuffer_tex0), gbuffer_tex1(gbuffer_tex1)
	{
		quad = new PlaneGeometry(2.0f, 2.0f);
		createProgram();
	}

	MeshDeferredRenderPass_Unpack::~MeshDeferredRenderPass_Unpack() {
		glDeleteProgram(program);
		quad->dispose();
	}

	void MeshDeferredRenderPass_Unpack::createProgram() {
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