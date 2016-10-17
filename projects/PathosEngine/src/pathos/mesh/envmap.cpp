#include <glm/gtx/transform.hpp>
#include <pathos/mesh/envmap.h>
#include <pathos/render/shader.h>

namespace pathos {

	/**
	* @param	textureID	texture id in which cubemap images are loaded. see pathos/loader/imageloader.h -> loadCubemapTexture().
	*/
	Skybox::Skybox(GLuint textureID) {
		this->textureID = textureID;
		
		string vshader = R"(#version 430 core
out VS_OUT { vec3 tc; } vs_out;
uniform mat4 viewTransform;
void main() {
  const vec3[4] vertices = vec3[4](vec3(-1,-1,1), vec3(1,-1,1), vec3(-1,1,1), vec3(1,1,1));
  vs_out.tc = mat3(viewTransform) * vertices[gl_VertexID];
  vs_out.tc.y *= -1;
  gl_Position = vec4(vertices[gl_VertexID], 1.0);
}
)";
		string fshader = R"(#version 430 core
layout (binding = 0) uniform samplerCube texCube;
in VS_OUT { vec3 tc; } fs_in;
layout (location = 0) out vec4 color;
void main() {
  color = texture(texCube, fs_in.tc);
}
)";
		program = createProgram(vshader, fshader);
	}

	void Skybox::activate(const glm::mat4& viewTransform) {
		glUseProgram(program);
		glm::mat4 transform = glm::scale(viewTransform, glm::vec3(1, 1, 1));
		glUniformMatrix4fv(glGetUniformLocation(program, "viewTransform"), 1, GL_FALSE, &transform[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	}

	void Skybox::render() {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

}