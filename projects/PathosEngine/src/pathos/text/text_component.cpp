#include "text_component.h"
// For static mesh proxy
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"

// Fallback font that must exist.
#define DEFAULT_FONT_TAG    "default"

namespace pathos {

	static const matrix4 invertTextY(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	TextMeshComponent::TextMeshComponent() {
		geom = std::make_unique<TextGeometry>();
		materialNew = std::unique_ptr<Material>(Material::createMaterialInstance("unlit_text"));
		materialOld = std::make_unique<AlphaOnlyTextureMaterial>(0);

		setFont(DEFAULT_FONT_TAG);
	}

	void TextMeshComponent::createRenderProxy(SceneProxy* scene) {
		if (text.size() == 0 || getVisibility() == false) {
			return;
		}

		// #todo-material-assembler-fatal: texcoord is only invalid in new material system?
		bool bUseMaterialShader = true;
		Material* material = bUseMaterialShader ? materialNew.get() : materialOld.get();

		StaticMeshProxy* proxy = ALLOC_RENDER_PROXY<StaticMeshProxy>(scene);
		proxy->doubleSided = true;
		proxy->renderInternal = false;
		proxy->modelMatrix = getLocalMatrix() * invertTextY;
		proxy->geometry = geom.get();
		proxy->material = material;
		proxy->bDepthPrepassNeedsFullVAO = true;
		// #todo-frustum-culling: Update worldBounds for text component

		if (bUseMaterialShader) {
			scene->proxyList_staticMeshTemp.push_back(proxy);
		} else {
			scene->proxyList_staticMesh[(uint8)materialOld->getMaterialID()].push_back(proxy);
		}
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
		materialNew->setConstantParameter("color", vector3(r, g, b));
		materialOld->setColor(r, g, b);
	}

	void TextMeshComponent::setFont(const std::string& tag) {
		bool validDesc = FontManager::get().getFontDesc(tag, fontDesc);
		if (!validDesc && tag != DEFAULT_FONT_TAG) {
			validDesc = FontManager::get().getFontDesc(DEFAULT_FONT_TAG, fontDesc);
		}
		CHECK(validDesc);
		GLuint fontCache = fontDesc.cacheTexture->getTexture();

		materialNew->setTextureParameter("fontCache", fontCache);
		materialOld->setTexture(fontCache);
	}

}
