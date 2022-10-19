#pragma once

#include "pathos/text/text_geometry.h"
#include "pathos/text/font_mgr.h"
#include "pathos/material/material.h"
#include "pathos/actor/scene_component.h"
#include "pathos/smart_pointer.h"

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
		uniquePtr<TextGeometry> geom;
		uniquePtr<Material> material;
		FontDesc fontDesc;

		bool dynamicDataDirty = false;

	};

}
