#include "overlaypass_standard.h"
#include "pathos/overlay/display_object_proxy.h"
#include "pathos/rhi/shader_program.h"

#include "badger/types/matrix_types.h"

namespace pathos {

	struct UBO_OverlayStandard {
		matrix4 transform;
		vector4 color;
	};

	class OverlayStandardVS : public ShaderStage {
	public:
		OverlayStandardVS() : ShaderStage(GL_VERTEX_SHADER, "OverlayStandardVS") {
			addDefine("VERTEX_SHADER 1");
			setFilepath("overlay_standard.glsl");
		}
	};

	class OverlayStandardFS : public ShaderStage {
	public:
		OverlayStandardFS() : ShaderStage(GL_FRAGMENT_SHADER, "OverlayStandardFS") {
			addDefine("FRAGMENT_SHADER 1");
			setFilepath("overlay_standard.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_OverlayStandard, OverlayStandardVS, OverlayStandardFS);
	
}

namespace pathos {

	OverlayPass_Standard::OverlayPass_Standard() {
		ubo.init<UBO_OverlayStandard>();
		rgba = vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void OverlayPass_Standard::renderOverlay(
		RenderCommandList& cmdList,
		DisplayObject2DProxy* object,
		const Transform& transformAccum)
	{
		MeshGeometry* geom = object->geometry;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_OverlayStandard);
		cmdList.useProgram(program.getGLName());

		bool bUseAlpha = rgba.a < 1.0f;
		if (bUseAlpha) {
			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
		}

		UBO_OverlayStandard uboData;
		uboData.transform = transformAccum.getMatrix();
		uboData.color = rgba;
		ubo.update(cmdList, 1, &uboData);

		geom->bindFullAttributesVAO(cmdList);
		geom->drawPrimitive(cmdList);
		geom->unbindVAO(cmdList);

		if (bUseAlpha) {
			cmdList.disable(GL_BLEND);
			cmdList.disable(GL_DEPTH_TEST);
		}
	}

}
