#include "render_device.h"
#include "gl_core.h"
#include "pathos/util/log.h"
#include "pathos/util/gl_context_manager.h"

#include <mutex>

#if GL_ERROR_CALLBACK
void glErrorCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		/* From glcorearb.h
		#define GL_DEBUG_SOURCE_API               0x8246
		#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
		#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
		#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
		#define GL_DEBUG_SOURCE_APPLICATION       0x824A
		#define GL_DEBUG_SOURCE_OTHER             0x824B
		#define GL_DEBUG_TYPE_ERROR               0x824C
		#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
		#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
		#define GL_DEBUG_TYPE_PORTABILITY         0x824F
		#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
		#define GL_DEBUG_TYPE_OTHER               0x8251
		#define GL_DEBUG_SEVERITY_HIGH            0x9146
		#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
		#define GL_DEBUG_SEVERITY_LOW             0x9148
		*/
		const char* sourceStrings[] = { "API", "WINDOW_SYSTEM", "SHADER_COMPILER", "THIRD_PARTY", "APPLICATION", "OTHER" };
		const char* typeStrings[] = { "ERROR", "DEPRECATED", "UNDEFINED", "PORTABILITY", "PERFORMANCE", "OTHER" };
		const char* severityStrings[] = { "HIGH", "MEDIUM", "LOW" };

		// #todo-fatal: Sometimes DeferredRenderer::clearGBuffer() fails and leads to here. (reason: gbufferFBO is incomplete)

		fprintf_s(stderr, "GL CALLBACK: renderCommandIx=%d, source=0x%x(%s), type=0x%x(%s), severity=0x%x(%s), message=%s\n",
			pathos::gRenderDevice->getImmediateCommandList().debugCurrentCommandIx,
			source, sourceStrings[source - 0x8246], type, typeStrings[type - 0x824C], severity, severityStrings[severity - 0x9146], message);
		__debugbreak();
	}
}
#endif

namespace pathos {

	OpenGLDevice* gRenderDevice = nullptr;

	void ENQUEUE_RENDER_COMMAND(std::function<void(RenderCommandList& commandList)> lambda) {
		//CHECK(isInMainThread());

		if (isInRenderThread()) {
			lambda(gRenderDevice->getImmediateCommandList());
		} else {
			gRenderDevice->getDeferredCommandList().registerHook(lambda);
		}
	}

	void FLUSH_RENDER_COMMAND(bool waitForGPU) {
		CHECK(isInMainThread());

		std::mutex flushMutex;
		std::condition_variable flushCondVar;
		std::unique_lock<std::mutex> cvLock(flushMutex);
		std::atomic<bool> alreadyFlushed = false;

		// #todo-renderthread-fatal: Is it safe to pass flushCondVar like this?
		gRenderDevice->getDeferredCommandList().registerHook([&flushCondVar, &alreadyFlushed, waitForGPU](RenderCommandList& cmdList) -> void {
			if (waitForGPU) {
				glFinish();
			}
			alreadyFlushed = true;
			flushCondVar.notify_all();
		});

		if (!alreadyFlushed) {
			flushCondVar.wait(cvLock);
		}
	}

	void TEMP_FLUSH_RENDER_COMMAND(bool waitForGPU) {
		CHECK(isInMainThread());

		std::mutex flushMutex;
		std::condition_variable flushCondVar;
		std::unique_lock<std::mutex> cvLock(flushMutex);
		std::atomic<bool> alreadyFlushed = false;

		// #todo-renderthread-fatal: Is it safe to pass flushCondVar like this?
		gRenderDevice->getDeferredCommandList().registerHook([&flushCondVar, &alreadyFlushed, waitForGPU](RenderCommandList& cmdList) -> void {
			if (waitForGPU) {
				glFinish();
			}
			alreadyFlushed = true;
			flushCondVar.notify_all();
		});

		if (!alreadyFlushed) {
			flushCondVar.wait(cvLock);
		}
	}

	OpenGLDevice::OpenGLDevice()
		: glObjectLabelMaxLength(-1)
	{
		CHECKF(gRenderDevice == nullptr, "Render device already exists");
		gRenderDevice = this;

		::memset(&extensionSupport, 0, sizeof(extensionSupport));
	}

	OpenGLDevice::~OpenGLDevice()
	{
	}

	bool OpenGLDevice::initialize()
	{
		if (gl3wInit()) {
			LOG(LogError, "Failed to initialize GL3W");
			return false;
		}
		if (!gl3wIsSupported(REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION)) {
			LOG(LogError, "GL %d.%d is not supported", REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION);
			return false;
		}

		checkExtensions();

		// #todo-renderthread: Wanna get rid of deferred_command_list :/
		// Create command lists
		immediate_command_list = std::make_unique<RenderCommandList>("immediate");
		deferred_command_list = std::make_unique<RenderCommandList>("deferred");
		hook_command_list = std::make_unique<RenderCommandList>("hook");
		immediate_command_list->setHookCommandList(hook_command_list.get());
		deferred_command_list->setHookCommandList(hook_command_list.get());

#if GL_ERROR_CALLBACK
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glErrorCallback, 0);
#endif

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glGetIntegerv(GL_MAX_LABEL_LENGTH, &glObjectLabelMaxLength);

		LOG(LogInfo, "GL version: %s", glGetString(GL_VERSION));
		LOG(LogInfo, "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		return true;
	}

}

// API for GPU resource creation and deletion. (not queued in command list)
// 
namespace pathos {

	void OpenGLDevice::genTextures(GLsizei n, GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glGenTextures(n, textures);
	}

	void OpenGLDevice::genQueries(GLsizei n, GLuint* queries) {
		CHECK_GL_CONTEXT_TAKEN();
		glGenQueries(n, queries);
	}

	void OpenGLDevice::createTextures(GLenum target, GLsizei n, GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateTextures(target, n, textures);
	}

	void OpenGLDevice::createFramebuffers(GLsizei n, GLuint* framebuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateFramebuffers(n, framebuffers);
	}

	void OpenGLDevice::createVertexArrays(GLsizei n, GLuint* arrays) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateVertexArrays(n, arrays);
	}

	void OpenGLDevice::createSamplers(GLsizei n, GLuint* samplers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateSamplers(n, samplers);
	}

	void OpenGLDevice::createQueries(GLenum target, GLsizei n, GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateQueries(target, n, ids);
	}

	void OpenGLDevice::createTransformFeedbacks(GLsizei n, GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateTransformFeedbacks(n, ids);
	}

	void OpenGLDevice::createBuffers(GLsizei n, GLuint* buffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateBuffers(n, buffers);
	}

	void OpenGLDevice::createRenderbuffers(GLsizei n, GLuint* renderbuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateRenderbuffers(n, renderbuffers);
	}

	void OpenGLDevice::createProgramPipelines(GLsizei n, GLuint* pipelines) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateProgramPipelines(n, pipelines);
	}

	void OpenGLDevice::deleteQueries(GLsizei n, const GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteQueries(n, ids);
	}

	void OpenGLDevice::deleteBuffers(GLsizei n, const GLuint* buffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteBuffers(n, buffers);
	}

	void OpenGLDevice::deleteProgram(GLuint program) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteProgram(program);
	}

	void OpenGLDevice::objectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label) {
		CHECK_GL_CONTEXT_TAKEN();
		CHECKF(::strlen(label) <= glObjectLabelMaxLength, "objectLabel is too long");
		glObjectLabel(identifier, name, length, label);
	}

	void OpenGLDevice::deleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteFramebuffers(n, framebuffers);
	}

	void OpenGLDevice::deleteVertexArrays(GLsizei n, const GLuint* arrays) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteVertexArrays(n, arrays);
	}

	void OpenGLDevice::deleteTextures(GLsizei n, const GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteTextures(n, textures);
	}

	GLint OpenGLDevice::getUniformLocation(GLuint program, const GLchar* name) {
		CHECK_GL_CONTEXT_TAKEN();
		return glGetUniformLocation(program, name);
	}

	void OpenGLDevice::checkExtensions() {
		CHECK_GL_CONTEXT_TAKEN();
		::memset(&extensionSupport, 0, sizeof(extensionSupport));

		GLint n;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n);

		std::vector<const char*> extNames(n, nullptr);
		for (GLint i = 0; i < n; ++i) {
			extNames[i] = (const char*)glGetStringi(GL_EXTENSIONS, i);
		}
		
		auto findExt = [&](const char* desiredExt) -> bool {
			for (GLint i = 0; i < n; ++i) {
				if (strcmp(extNames[i], desiredExt)) {
					return true;
				}
			}
			return false;
		};

		// #todo-gl-extension: Utilize available extensions
		extensionSupport.NV_ray_tracing                  = findExt("GL_NV_ray_tracing");
		extensionSupport.NV_mesh_shader                  = findExt("GL_NV_mesh_shader");
		extensionSupport.NV_shading_rate_image           = findExt("GL_NV_shading_rate_image");
		extensionSupport.NV_shader_texture_footprint     = findExt("GL_NV_shader_texture_footprint");
		extensionSupport.NV_representative_fragment_test = findExt("GL_NV_representative_fragment_test");
		extensionSupport.NV_fragment_shader_barycentric  = findExt("GL_NV_fragment_shader_barycentric");
		extensionSupport.NV_compute_shader_derivatives   = findExt("GL_NV_compute_shader_derivatives");
		extensionSupport.NV_scissor_exclusive            = findExt("GL_NV_scissor_exclusive");
	}

}
