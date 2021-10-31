#pragma once

#include "pathos/mesh/mesh.h"
#include "pathos/text/font_texture_cache.h"
#include "pathos/render/render_command_list.h"

#include "gl_core.h"
#include <string>
#include <map>

namespace pathos {

	// #todo-text: Bring TextMesh back. It was for scene rendering, and Label is for UI rendering.
	//class TextMesh : public Mesh {
	//	//
	//};

	class TextGeometry : public MeshGeometry {

	public:
		void configure(RenderCommandList& cmdList, FontTextureCache& cache, const std::wstring& text);

	private:
		// #todo-text: Redundant with member variables of the superclass.
		std::vector<GLfloat> positions;
		std::vector<GLfloat> normals;
		std::vector<GLfloat> uvs;
		std::vector<GLuint> indices;

	};

}
