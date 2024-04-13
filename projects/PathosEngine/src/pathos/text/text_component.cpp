#include "text_component.h"
// For static mesh proxy
#include "pathos/scene/static_mesh_component.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_proxy.h"

// Fallback font that must exist.
#define DEFAULT_FONT_TAG    "default"

namespace pathos {

	static const matrix4 invertTextY(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	static AABB calculateWorldBounds(const AABB& localBounds, const matrix4& localToWorld) {
		vector3 minB = localBounds.minBounds;
		vector3 maxB = localBounds.maxBounds;
		vector3 vs[8];
		for (uint32 i = 0; i < 8; ++i) {
			vs[i].x = (i & 1) ? minB.x : maxB.x;
			vs[i].y = ((i >> 1) & 1) ? minB.y : maxB.y;
			vs[i].z = ((i >> 2) & 1) ? minB.z : maxB.z;
		}
		minB = vector3(std::numeric_limits<float>::max());
		maxB = vector3(std::numeric_limits<float>::lowest());
		for (uint32 i = 0; i < 8; ++i) {
			vs[i] = vector3(localToWorld * vector4(vs[i], 1.0f));
			minB = glm::min(minB, vs[i]);
			maxB = glm::max(maxB, vs[i]);
		}
		return AABB::fromMinMax(minB, maxB);
	}

	TextMeshComponent::TextMeshComponent() {
		geom = makeUnique<TextGeometry>();
		material = uniquePtr<Material>(Material::createMaterialInstance("unlit_text"));

		setFont(DEFAULT_FONT_TAG);
	}

	void TextMeshComponent::createRenderProxy(SceneProxy* scene) {
		if (text.size() == 0 || getVisibility() == false) {
			return;
		}

		StaticMeshProxy* proxy = ALLOC_RENDER_PROXY<StaticMeshProxy>(scene);
		proxy->doubleSided = true;
		proxy->renderInternal = false;
		proxy->modelMatrix = getLocalMatrix() * invertTextY;
		proxy->prevModelMatrix = proxy->modelMatrix; // #todo-motion-blur
		proxy->geometry = geom.get();
		proxy->material = material.get();
		// #todo-frustum-culling: Proper worldBounds for text component
		proxy->worldBounds = calculateWorldBounds(AABB::fromCenterAndHalfSize(vector3(0.0f), vector3(1.0f)), proxy->modelMatrix);

		scene->addStaticMeshProxy(proxy);
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
		material->setConstantParameter("color", vector3(r, g, b));
	}

	void TextMeshComponent::setFont(const std::string& tag) {
		bool validDesc = FontManager::get().getFontDesc(tag, fontDesc);
		if (!validDesc && tag != DEFAULT_FONT_TAG) {
			validDesc = FontManager::get().getFontDesc(DEFAULT_FONT_TAG, fontDesc);
		}
		CHECK(validDesc);

		Texture* fontCache = fontDesc.cacheTexture->getTexture();
		material->setTextureParameter("fontCache", fontCache);
	}

}
