#include "text_component.h"
// For static mesh proxy
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"

// Fallback font that must exist.
#define DEFAULT_FONT_TAG    "default"

namespace pathos {

	static matrix4 invertTextY(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	TextMeshComponent::TextMeshComponent() {
		geom = std::make_unique<TextGeometry>();
		material = std::make_unique<AlphaOnlyTextureMaterial>(0);

		setFont(DEFAULT_FONT_TAG);
	}

	void TextMeshComponent::createRenderProxy(SceneProxy* scene) {
		if (text.size() == 0 || getVisibility() == false) {
			return;
		}

		uint16 materialID = static_cast<uint16>(material->getMaterialID());
		
		StaticMeshProxy* proxy = ALLOC_RENDER_PROXY<StaticMeshProxy>(scene);
		proxy->doubleSided = true;
		proxy->renderInternal = false;
		proxy->modelMatrix = getMatrix() * invertTextY;
		proxy->geometry = geom.get();
		proxy->material = material.get();

		scene->proxyList_staticMesh[materialID].push_back(proxy);
	}

	void TextMeshComponent::updateDynamicData_renderThread(RenderCommandList& cmdList) {
		if (dynamicDataDirty) {
			geom->configure(cmdList, *fontDesc.cacheTexture, text);
			dynamicDataDirty = false;
		}
	}

	void TextMeshComponent::setText(const wchar_t* inText) {
		text = inText;
		dynamicDataDirty = true;
	}

	void TextMeshComponent::setColor(float r, float g, float b) {
		material->setColor(r, g, b);
	}

	void TextMeshComponent::setFont(const std::string& tag) {
		bool validDesc = FontManager::get().getFontDesc(tag, fontDesc);
		if (!validDesc && tag != DEFAULT_FONT_TAG) {
			validDesc = FontManager::get().getFontDesc(DEFAULT_FONT_TAG, fontDesc);
		}
		CHECK(validDesc);

		material->setTexture(fontDesc.cacheTexture->getTexture());
	}

}
