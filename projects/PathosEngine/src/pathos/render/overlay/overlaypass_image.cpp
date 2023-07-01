#include "overlaypass_image.h"
#include "pathos/overlay/display_object_proxy.h"
#include "pathos/rhi/shader_program.h"

#include "badger/types/matrix_types.h"

namespace pathos {

	static constexpr GLuint TEXTURE_UNIT_IMAGE = 0;

	struct UBO_OverlayImage {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4 transform;
	};

	class OverlayImageVS : public ShaderStage {
	public:
		OverlayImageVS() : ShaderStage(GL_VERTEX_SHADER, "OverlayImageVS") {
			addDefine("VERTEX_SHADER 1");
			setFilepath("overlay_image.glsl");
		}
	};

	class OverlayImageFS : public ShaderStage {
	public:
		OverlayImageFS() : ShaderStage(GL_FRAGMENT_SHADER, "OverlayImageFS") {
			addDefine("FRAGMENT_SHADER 1");
			setFilepath("overlay_image.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_OverlayImage, OverlayImageVS, OverlayImageFS);

}

namespace pathos {

	OverlayPass_Image::OverlayPass_Image() {
		ubo.init<UBO_OverlayImage>("UBO_OverlayImage");
	}

	void OverlayPass_Image::renderOverlay(
		RenderCommandList& cmdList,
		DisplayObject2DProxy* object,
		const Transform& transformAccum)
	{
		MeshGeometry* geom = object->geometry;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_OverlayImage);
		cmdList.useProgram(program.getGLName());

		cmdList.enable(GL_BLEND);
		cmdList.blendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

		UBO_OverlayImage uboData;
		uboData.transform = transformAccum.getMatrix();
		ubo.update(cmdList, UBO_OverlayImage::BINDING_POINT, &uboData);

		cmdList.bindTextureUnit(TEXTURE_UNIT_IMAGE, textureName);

		geom->activate_position_uv(cmdList);
		geom->activateIndexBuffer(cmdList);
		geom->drawPrimitive(cmdList);
		geom->deactivate(cmdList);
		geom->deactivateIndexBuffer(cmdList);

		cmdList.disable(GL_BLEND);
		cmdList.disable(GL_DEPTH_TEST);
	}

}
