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

namespace pathos {

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

		{
			const std::vector<StaticMeshProxy*>& proxyList = scene->getOpaqueStaticMeshes();
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
				bool bUseWireframeMode = material->bWireframe;
				currentProgramHash = materialShader->programHash;
				currentMIID = material->internal_getMaterialInstanceID();

				if (bShouldBindProgram) {
					SCOPED_DRAW_EVENT(BindMaterialProgram);

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

				// #todo-material-assembler: How to detect if binding textures is mandatory?
				// Example cases:
				// - The vertex shader uses VTF(Vertex Texture Fetch)
				// - The pixel shader uses discard
				if (bShouldUpdateMaterialParameters) {
					for (const MaterialTextureParameter& mtp : material->internal_getTextureParameters()) {
						cmdList.bindTextureUnit(mtp.binding, mtp.glTexture);
					}
				}

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided || bUseWireframeMode) cmdList.disable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

				// #todo-material-assembler: How to detect if a VS uses vertex buffers other than the position buffer?
				if (material->bTrivialDepthOnlyPass) {
					proxy->geometry->activate_position(cmdList);
				} else {
					proxy->geometry->activate_position_uv_normal_tangent_bitangent(cmdList);
				}

				proxy->geometry->activateIndexBuffer(cmdList);
				proxy->geometry->drawPrimitive(cmdList);

				// #todo-renderer: Batching by same state
				if (proxy->doubleSided || bUseWireframeMode) cmdList.enable(GL_CULL_FACE);
				if (proxy->renderInternal) cmdList.frontFace(GL_CCW);
				if (bUseWireframeMode) cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
	}

}
