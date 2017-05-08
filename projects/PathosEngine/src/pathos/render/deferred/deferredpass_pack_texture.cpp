#include "deferredpass_pack_texture.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	MeshDeferredRenderPass_Pack_FlatTexture::MeshDeferredRenderPass_Pack_FlatTexture() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::createProgram() {
		string vshader = R"(#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out VS_OUT {
	vec3 ws_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} vs_out;

uniform mat4 modelTransform;
uniform mat4 mvpTransform;

void main() {
	vs_out.ws_coords = (modelTransform * vec4(position, 1.0f)).xyz;
	vs_out.normal = mat3(modelTransform) * normal;
	vs_out.tangent = vec3(0);
	vs_out.texcoord = uv;
	vs_out.material_id = 2;

	gl_Position = mvpTransform * vec4(position, 1.0f);
}

)";
		string fshader = R"(#version 430 core

layout (location = 0) out uvec4 output0;
layout (location = 1) out vec4 output1;

uniform sampler2D tex_diffuse;

in VS_OUT {
	vec3 ws_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} fs_in;

void main() {
	uvec4 outvec0 = uvec4(0);
	vec4 outvec1 = vec4(0);

	vec3 color = texture(tex_diffuse, fs_in.texcoord).rgb;

	outvec0.x = packHalf2x16(color.xy);
	outvec0.y = packHalf2x16(vec2(color.z, fs_in.normal.x));
	outvec0.z = packHalf2x16(fs_in.normal.yz);
	outvec0.w = fs_in.material_id;
	outvec1.xyz = fs_in.ws_coords;
	outvec1.w = 128.0;

	output0 = outvec0;
	output1 = outvec1;
}

)";

		program = pathos::createProgram(vshader, fshader);
		positionLocation = 0;
		uvLocation = 1;
		normalLocation = 2;
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		TextureMaterial* material = static_cast<TextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activatePositionBuffer(positionLocation);
		geometry->activateUVBuffer(uvLocation);
		geometry->activateNormalBuffer(normalLocation);
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// uniform: transform
		glm::mat4& mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(mvpMatrix));

		// texture binding
		glActiveTexture(GL_TEXTURE0 + MeshDeferredRenderPass_Pack_FlatTexture::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getTexture());
		glUniform1i(glGetUniformLocation(program, "tex_diffuse"), TEXTURE_UNIT);

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivatePositionBuffer(positionLocation);
		geometry->deactivateUVBuffer(uvLocation);
		geometry->deactivateNormalBuffer(normalLocation);
		geometry->deactivateIndexBuffer();

		glActiveTexture(GL_TEXTURE0 + MeshDeferredRenderPass_Pack_FlatTexture::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

}