#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/mesh/mesh.h"
#include "pathos/actor/scene_component.h"
#include "pathos/text/font_texture_cache.h"
#include "pathos/render/render_command_list.h"

#include <string>
#include <map>

namespace pathos {

	class TextGeometry : public MeshGeometry {

	public:
		void configure(
			RenderCommandList& cmdList,
			FontTextureCache& cache,
			const std::wstring& text);
	
	};

}
