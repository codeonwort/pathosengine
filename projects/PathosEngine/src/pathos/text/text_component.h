#pragma once

#include "pathos/text/text_geometry.h"
#include "pathos/text/font_mgr.h"
#include "pathos/material/material.h"
#include "pathos/actor/scene_component.h"
#include <memory>

namespace pathos {

	class TextMeshComponent : public SceneComponent {

	public:
		TextMeshComponent();
		~TextMeshComponent() = default;

		virtual void createRenderProxy(Scene* scene);
		virtual void updateDynamicData_renderThread(RenderCommandList& cmdList);

		void setText(const wchar_t* text);
		void setColor(float r, float g, float b);
		void setFont(const std::string& tag);

	private:
		std::wstring text;
		std::unique_ptr<TextGeometry> geom;
		std::unique_ptr<AlphaOnlyTextureMaterial> material;
		FontDesc fontDesc;

		bool dynamicDataDirty = false;

	};

}
