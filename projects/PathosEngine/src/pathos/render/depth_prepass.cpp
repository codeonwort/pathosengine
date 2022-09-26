#include "depth_prepass.h"
#include "render_device.h"
#include "scene_render_targets.h"
#include "scene_proxy.h"
#include "pathos/engine_policy.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader_program.h"
#include "pathos/shader/material_shader.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/material/material.h"
#include "pathos/console.h"

// #todo-material
#define SUPPORT_ALPHAONLY_DISCARD 0

namespace pathos {

#if SUPPORT_ALPHAONLY_DISCARD
	static constexpr uint32 ALPHAONLY_TEXTURE_UNIT = 1;
#endif

	struct UBO_DepthPrepass {
		matrix4 mvTransform;
		matrix3x4 mvMatrix3x3;
		vector4 billboardParam;
	};

	// #todo-material-assembler: Remove this
	struct UBO_DepthPrepass_PerObject {
		static constexpr uint32 BINDING_POINT = 1;
		matrix4 modelTransform;
		matrix4 mvTransform;
		matrix3x4 mvMatrix3x3;
	};

	class DepthPrepassVS : public ShaderStage {
	public:
		DepthPrepassVS() : ShaderStage(GL_VERTEX_SHADER, "DepthPrepassVS")
		{
			addDefine("VERTEX_SHADER", 1);
			setFilepath("depth_prepass.glsl");
		}
	};

	class DepthPrepassFS : public ShaderStage {
	public:
		DepthPrepassFS() : ShaderStage(GL_FRAGMENT_SHADER, "DepthPrepassFS")
		{
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("depth_prepass.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_DepthPrepass, DepthPrepassVS, DepthPrepassFS);

}

namespace pathos {

	void DepthPrepass::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_DepthPrepass");

		ubo.init<UBO_DepthPrepass>();
		uboPerObject.init<UBO_DepthPrepass_PerObject>();
	}

	void DepthPrepass::destroyResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void DepthPrepass::renderPreDepth(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(DepthPrepass);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DepthPrepass);
		cmdList.useProgram(program.getGLName());

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
			cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
			cmdList.depthFunc(GL_GREATER);
		} else {
			cmdList.depthFunc(GL_LESS);
		}
		cmdList.enable(GL_DEPTH_TEST);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferDrawBuffers(fbo, 0, nullptr);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		GLfloat* sceneDepthClearValue = (GLfloat*)cmdList.allocateSingleFrameMemory(sizeof(GLfloat));
		*sceneDepthClearValue = pathos::getDeviceFarDepth();
		cmdList.clearNamedFramebufferfv(fbo, GL_DEPTH, 0, sceneDepthClearValue);

		static ConsoleVariableBase* cvarFrustum = ConsoleVariableManager::get().find("r.frustum_culling");
		CHECK(cvarFrustum != nullptr);
		const bool bEnableFrustumCulling = cvarFrustum->getInt() != 0;

		for (uint8 i = 0; i < (uint8)(MATERIAL_ID::NUM_MATERIAL_IDS); ++i) {
			const auto& proxyList = scene->proxyList_staticMesh[i];
			for (StaticMeshProxy* proxy : proxyList) {

				if (proxy->material->getMaterialID() == MATERIAL_ID::TRANSLUCENT_SOLID_COLOR) {
					continue;
				}

				if (bEnableFrustumCulling && !proxy->bInFrustum) {
					continue;
				}

				if (proxy->material->internal_getMaterialShader() != nullptr) {
					continue;
				}

				if (proxy->material->getMaterialID() == MATERIAL_ID::PBR_TEXTURE) {
					if (static_cast<PBRTextureMaterial*>(proxy->material)->writeAllPixels == false) {
						continue;
					}
				}

				// Render state modifiers
				bool doubleSided = proxy->doubleSided;
				bool renderInternal = proxy->renderInternal;
				bool wireframe = proxy->material->getMaterialID() == MATERIAL_ID::WIREFRAME;

				if (doubleSided) cmdList.disable(GL_CULL_FACE);
				if (renderInternal) cmdList.frontFace(GL_CW);
				if (wireframe) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

				// #todo-material: temp billboard
				ColorMaterial* colorMaterial = nullptr;
				if (proxy->material->getMaterialID() == MATERIAL_ID::SOLID_COLOR) {
					colorMaterial = static_cast<ColorMaterial*>(proxy->material);
				}

#if SUPPORT_ALPHAONLY_DISCARD
				// #todo-material: temp alphaonly processing in prepass
				if (proxy->material->getMaterialID() == MATERIAL_ID::ALPHA_ONLY_TEXTURE) {
					AlphaOnlyTextureMaterial* M = static_cast<AlphaOnlyTextureMaterial*>(proxy->material);
					cmdList.bindTextureUnit(ALPHAONLY_TEXTURE_UNIT, M->getTexture());
				}
#endif

				UBO_DepthPrepass uboData;
				{
					uboData.mvTransform = camera->getViewMatrix() * proxy->modelMatrix;
					uboData.mvMatrix3x3 = matrix3x4(uboData.mvTransform);
					uboData.billboardParam.x = colorMaterial && colorMaterial->billboard ? 1.0f : 0.0f;
					uboData.billboardParam.y = colorMaterial ? colorMaterial->billboardWidth : 0.0f;
				}
				ubo.update(cmdList, 1, &uboData);

				if (colorMaterial && colorMaterial->billboard) {
					proxy->geometry->activate_position_uv(cmdList);
				} else {
					proxy->geometry->activate_position(cmdList);
				}
				proxy->geometry->activateIndexBuffer(cmdList);
				proxy->geometry->drawPrimitive(cmdList);
				proxy->geometry->deactivate(cmdList);
				proxy->geometry->deactivateIndexBuffer(cmdList);

				if (doubleSided) cmdList.enable(GL_CULL_FACE);
				if (renderInternal) cmdList.frontFace(GL_CCW);
				if (wireframe) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		// #todo-material-assembler: Render prepass with new material system
		{
			std::vector<StaticMeshProxy*>& proxyList = scene->proxyList_staticMeshTemp;
			const size_t numProxies = proxyList.size();
			uint32 currentProgramHash = 0;
			uint32 currentMIID = 0xffffffff;

			for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
				StaticMeshProxy* proxy = proxyList[proxyIx];
				Material* material = proxy->material;
				MaterialShader* materialShader = material->internal_getMaterialShader();

				// Early out
				if (bEnableFrustumCulling && !proxy->bInFrustum) {
					continue;
				}

				bool bShouldBindProgram = (currentProgramHash != materialShader->programHash);
				bool bShouldUpdateMaterialParameters = bShouldBindProgram || (currentMIID != material->internal_getMaterialInstanceID());
				currentProgramHash = materialShader->programHash;
				currentMIID = material->internal_getMaterialInstanceID();

				if (bShouldBindProgram) {
					//SCOPED_DRAW_EVENT(BindMaterialProgram);
					uint32 programName = materialShader->program->getGLName();
					CHECK(programName != 0 && programName != 0xffffffff);
					cmdList.useProgram(programName);
				}

				// Update UBO (per object)
				{
					UBO_DepthPrepass_PerObject uboData;
					uboData.modelTransform = proxy->modelMatrix;
					uboData.mvTransform = camera->getViewMatrix() * proxy->modelMatrix;
					uboData.mvMatrix3x3 = matrix3x4(uboData.mvTransform);
					uboPerObject.update(cmdList, UBO_DepthPrepass_PerObject::BINDING_POINT, &uboData);
				}

				// Update UBO (material)
				if (bShouldUpdateMaterialParameters && materialShader->uboTotalBytes > 0) {
					uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(materialShader->uboTotalBytes));
					material->internal_fillUniformBuffer(uboMemory);
					materialShader->uboMaterial.update(cmdList, materialShader->uboBindingPoint, uboMemory);
				}

				// #todo-material-assembler: Bind texture units like base pass?
				// How to detect if a vertex shader uses VTF(Vertex Texture Fetch)?
				// ...

				// #todo-material-assembler: How to detect if a VS uses vertex buffers other than the position buffer?
				proxy->geometry->activate_position(cmdList);

				proxy->geometry->activateIndexBuffer(cmdList);
				proxy->geometry->drawPrimitive(cmdList);
			}
		}
	}

}
