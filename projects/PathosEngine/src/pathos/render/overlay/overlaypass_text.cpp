#include "overlaypass_text.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/display_object_proxy.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"

namespace pathos {

	static constexpr GLuint FONT_TEXTURE_UNIT = 0;

	struct UBO_OverlayText {
		static constexpr GLuint BINDING_POINT = 1;

		matrix4 transform;
		vector4 color;
	};

	class OverlayTextVS : public ShaderStage {
	public:
		OverlayTextVS() : ShaderStage(GL_VERTEX_SHADER, "OverlayTextVS") {
			addDefine("VERTEX_SHADER 1");
			setFilepath("overlay_text.glsl");
		}
	};

	class OverlayTextFS : public ShaderStage {
	public:
		OverlayTextFS() : ShaderStage(GL_FRAGMENT_SHADER, "OverlayTextFS") {
			addDefine("FRAGMENT_SHADER 1");
			setFilepath("overlay_text.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_OverlayText, OverlayTextVS, OverlayTextFS);

}

namespace pathos {

	OverlayPass_Text::OverlayPass_Text() {
		ubo.init<UBO_OverlayText>();
	}

	void OverlayPass_Text::renderOverlay(
		RenderCommandList& cmdList,
		DisplayObject2DProxy* object,
		const Transform& transformAccum)
	{
		LabelProxy* label = static_cast<LabelProxy*>(object);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_OverlayText);
		cmdList.useProgram(program.getGLName());

		MeshGeometry* geom = object->geometry;

		// uniform
		UBO_OverlayText uboData;
		uboData.transform = transformAccum.getMatrix();
		uboData.color = color;
		ubo.update(cmdList, UBO_OverlayText::BINDING_POINT, &uboData);

		// shader resources
		cmdList.bindTextureUnit(FONT_TEXTURE_UNIT, label->fontDesc.cacheTexture->getTexture()->internal_getGLName());

		cmdList.enable(GL_BLEND);
		cmdList.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		geom->activate_position_uv(cmdList);
		geom->activateIndexBuffer(cmdList);
		geom->drawPrimitive(cmdList);
		geom->deactivate(cmdList);
		geom->deactivateIndexBuffer(cmdList);
		
		cmdList.disable(GL_BLEND);
	}

}
