#include <pathos/mesh/envmap.h>
#include <pathos/render/shader.h>

namespace pathos {

	/**
	* @param	textureID	texture id in which cubemap images are loaded. see pathos/mesh/imageloader.h -> loadCubemapTexture().
	*/
	Skybox::Skybox(GLuint textureID) {
		this->textureID = textureID;
		
		string vshader = R"(#version 430 core
out VS_OUT { vec3 tc; } vs_out;
uniform mat4 viewTransform;
void main() {
  const vec3[4] vertices = vec3[4](vec3(-1,-1,1), vec3(1,-1,1), vec3(-1,1,1), vec3(1,1,1));
  vs_out.tc = mat3(viewTransform) * vertices[gl_VertexID];
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
		glUniformMatrix4fv(glGetUniformLocation(program, "viewTransform"), 1, GL_FALSE, &viewTransform[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	}

	void Skybox::render() {
		glDepthFunc(GL_LEQUAL);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

}