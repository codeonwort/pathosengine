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

		virtual void createRenderProxy(SceneProxy* scene) override;
		virtual void updateDynamicData_renderThread(RenderCommandList& cmdList) override;

		void setText(const wchar_t* text);
		void setColor(float r, float g, float b);
		void setFont(const std::string& tag);

	private:
		std::wstring text;
		std::unique_ptr<TextGeometry> geom;
		std::unique_ptr<Material> materialNew;
		std::unique_ptr<AlphaOnlyTextureMaterial> materialOld;
		FontDesc fontDesc;

		bool dynamicDataDirty = false;

	};

}
