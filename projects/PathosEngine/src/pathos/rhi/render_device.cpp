#include "render_device.h"
#include "pathos/rhi/gl_context_manager.h"
#include "pathos/rhi/gl_live_objects.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/buffer.h"
#include "pathos/util/log.h"
#include "pathos/console.h"

#include <mutex>

#define GL_ERROR_CALLBACK 1

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

		fprintf_s(stderr, "GL CALLBACK: renderCommandIx=%d, source=0x%x(%s), type=0x%x(%s), severity=0x%x(%s), message=%s\n",
			pathos::gRenderDevice->getImmediateCommandList().debugCurrentCommandIx,
			source, sourceStrings[source - 0x8246], type, typeStrings[type - 0x824C], severity, severityStrings[severity - 0x9146], message);
		__debugbreak();
	}
}
#endif

namespace pathos {

	OpenGLDevice* gRenderDevice = nullptr;

	// NOTE: Should be set in EngineConfig.ini to be effective.
	static ConsoleVariable<int32> cvarDumpGLDevice("r.dumpGLDevice", 0, "(read only) Dump GL device info to log/device_dump");
	static ConsoleVariable<int32> cvarPositionBufferPoolSize("r.positionBufferPoolSize", 32 * 1024 * 1024, "(read only) Size of global position buffer pool in bytes");
	static ConsoleVariable<int32> cvarVaryingBufferPoolSize("r.varyingBufferPoolSize", 64 * 1024 * 1024, "(read only) Size of global varying buffer pool in bytes");
	static ConsoleVariable<int32> cvarIndexBufferPoolSize("r.indexBufferPoolSize", 32 * 1024 * 1024, "(read only) Size of global vertex buffer pool in bytes");

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

	OpenGLDevice::OpenGLDevice() {
		CHECKF(gRenderDevice == nullptr, "Render device already exists");
		gRenderDevice = this;
		gGLLiveObjects = new GLLiveObjects;

		::memset(&extensionSupport, 0, sizeof(extensionSupport));
	}

	OpenGLDevice::~OpenGLDevice() {
		positionBufferPool->releaseGPUResource();
		varyingBufferPool->releaseGPUResource();
		indexBufferPool->releaseGPUResource();
		delete gGLLiveObjects;
	}

	bool OpenGLDevice::initialize()
	{
		if (gl3wInit()) {
			LOG(LogError, "[RenderDevice] Failed to initialize GL3W");
			return false;
		}
		if (!gl3wIsSupported(REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION)) {
			LOG(LogError, "[RenderDevice] GL %d.%d is not supported", REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION);
			return false;
		}

		checkExtensions();
		queryCapabilities();

		// #todo-renderthread: Wanna get rid of deferred_command_list :/
		// Create command lists
		immediate_command_list = makeUnique<RenderCommandList>("immediate");
		deferred_command_list = makeUnique<RenderCommandList>("deferred");
		hook_command_list = makeUnique<RenderCommandList>("hook");
		immediate_command_list->setHookCommandList(hook_command_list.get());
		deferred_command_list->setHookCommandList(hook_command_list.get());

#if GL_ERROR_CALLBACK
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glErrorCallback, 0);
#endif

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		LOG(LogInfo, "[RenderDevice] GL version: %s", glGetString(GL_VERSION));
		LOG(LogInfo, "[RenderDevice] GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		if (capabilities.bMemoryInfoAvailable) {
			LOG(LogInfo, "[RenderDevice] VRAM: %d MiB (available: %d MiB)",
				capabilities.dedicatedVideoMemoryMiB, capabilities.dedicatedVideoMemoryAvailableMiB);
		} else {
			LOG(LogInfo, "[RenderDevice] VRAM: unknown (Both 'NVX_gpu_memory_info' and 'ATI_meminfo' extensions are missing)");
		}

		positionBufferPool = new BufferPool;
		positionBufferPool->createGPUResource(cvarPositionBufferPoolSize.getInt(), "GPositionBufferPool");
		varyingBufferPool = new BufferPool;
		varyingBufferPool->createGPUResource(cvarVaryingBufferPoolSize.getInt(), "GVaryingBufferPool");
		indexBufferPool = new BufferPool;
		indexBufferPool->createGPUResource(cvarIndexBufferPoolSize.getInt(), "GIndexBufferPool");

		return true;
	}

	void OpenGLDevice::memreport(int64& outTotalBufferMemory, int64& outTotalTextureMemory) {
		gGLLiveObjects->memreport(outTotalBufferMemory, outTotalTextureMemory);
	}

	void OpenGLDevice::reportLiveObjects() {
		gGLLiveObjects->reportLiveObjects();
	}

	void OpenGLDevice::queryCapabilities() {
		glGetIntegerv(GL_MAX_LABEL_LENGTH, &capabilities.glObjectLabelMaxLength);
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &capabilities.glMaxUniformBlockSize);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &capabilities.glMaxComputeWorkGroupInvocations);
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &capabilities.glMaxComputeSharedMemorySize);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, capabilities.glMaxComputeWorkGroupCount + 0);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, capabilities.glMaxComputeWorkGroupCount + 1);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, capabilities.glMaxComputeWorkGroupCount + 2);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, capabilities.glMaxComputeWorkGroupSize + 0);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, capabilities.glMaxComputeWorkGroupSize + 1);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, capabilities.glMaxComputeWorkGroupSize + 2);

		if (extensionSupport.NVX_gpu_memory_info != 0) {
			// https://registry.khronos.org/OpenGL/extensions/NVX/NVX_gpu_memory_info.txt
			capabilities.bMemoryInfoAvailable = true;

			const GLenum GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          = 0x9047;
			const GLenum GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    = 0x9048;
			const GLenum GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  = 0x9049;
			const GLenum GPU_MEMORY_INFO_EVICTION_COUNT_NVX            = 0x904A;
			const GLenum GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            = 0x904B;

			glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &capabilities.dedicatedVideoMemoryKiB);
			glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &capabilities.dedicatedVideoMemoryAvailableKiB);
			capabilities.dedicatedVideoMemoryMiB = capabilities.dedicatedVideoMemoryKiB >> 10;
			capabilities.dedicatedVideoMemoryAvailableMiB = capabilities.dedicatedVideoMemoryAvailableKiB >> 10;
		} else if (extensionSupport.ATI_meminfo != 0) {
			// https://registry.khronos.org/OpenGL/extensions/ATI/ATI_meminfo.txt
			capabilities.bMemoryInfoAvailable = true;

			const GLenum VBO_FREE_MEMORY_ATI                           = 0x87FB;
			const GLenum TEXTURE_FREE_MEMORY_ATI                       = 0x87FC;
			const GLenum RENDERBUFFER_FREE_MEMORY_ATI                  = 0x87FD;

			// All values are in KiB
			// param[0] - total memory free in the pool
			// param[1] - largest available free block in the pool
			// param[2] - total auxiliary memory free
			// param[3] - largest auxiliary free block
			GLint vboParams[4], texParams[4], rbParams[4];
			glGetIntegerv(VBO_FREE_MEMORY_ATI, vboParams);
			glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, texParams);
			glGetIntegerv(RENDERBUFFER_FREE_MEMORY_ATI, rbParams);

			capabilities.dedicatedVideoMemoryKiB = std::max(vboParams[0], std::max(texParams[0], rbParams[0]));
			capabilities.dedicatedVideoMemoryMiB = capabilities.dedicatedVideoMemoryKiB >> 10;
			capabilities.dedicatedVideoMemoryAvailableKiB = std::max(vboParams[1], std::max(texParams[1], rbParams[1]));
			capabilities.dedicatedVideoMemoryAvailableMiB = capabilities.dedicatedVideoMemoryAvailableKiB >> 10;
		}
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

		if (cvarDumpGLDevice.getInt() != 0) {
			std::string basedir = pathos::getSolutionDir();
			basedir += "log/device_dump/";
			pathos::createDirectory(basedir.c_str());

			std::string dumpPath = basedir + "GL_extensions.txt";

			std::fstream fs(dumpPath, std::fstream::out);
			if (fs.is_open()) {
				for (uint32 i = 0; i < (uint32)extNames.size(); ++i) {
					fs << extNames[i] << '\n';
				}
				fs.close();
			}
		}
		
		auto findExt = [&](const char* desiredExt) -> bool {
			for (GLint i = 0; i < n; ++i) {
				if (0 == strcmp(extNames[i], desiredExt)) {
					return true;
				}
			}
			return false;
		};

		// #todo-gl-extension: Utilize available extensions
		extensionSupport.ATI_meminfo                          = findExt("GL_ATI_meminfo");
		extensionSupport.NV_ray_tracing                       = findExt("GL_NV_ray_tracing");
		extensionSupport.NV_mesh_shader                       = findExt("GL_NV_mesh_shader");
		extensionSupport.NV_shading_rate_image                = findExt("GL_NV_shading_rate_image");
		extensionSupport.NV_shader_texture_footprint          = findExt("GL_NV_shader_texture_footprint");
		extensionSupport.NV_representative_fragment_test      = findExt("GL_NV_representative_fragment_test");
		extensionSupport.NV_fragment_shader_barycentric       = findExt("GL_NV_fragment_shader_barycentric");
		extensionSupport.NV_compute_shader_derivatives        = findExt("GL_NV_compute_shader_derivatives");
		extensionSupport.NV_scissor_exclusive                 = findExt("GL_NV_scissor_exclusive");
		extensionSupport.NVX_gpu_memory_info                  = findExt("GL_NVX_gpu_memory_info");
		extensionSupport.NV_gpu_shader5                       = findExt("GL_NV_gpu_shader5");
		extensionSupport.EXT_shader_16bit_storage             = findExt("GL_EXT_shader_16bit_storage");
		extensionSupport.EXT_shader_explicit_arithmetic_types = findExt("GL_EXT_shader_explicit_arithmetic_types");
	}

}

// API for GPU resource creation and deletion. (not queued in command list)
namespace pathos {

	// CREATE -----------------------------------------------------------------

	void OpenGLDevice::genTextures(GLsizei n, GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glGenTextures(n, textures);
		gGLLiveObjects->genTextures(n, textures);
	}

	void OpenGLDevice::genQueries(GLsizei n, GLuint* queries) {
		CHECK_GL_CONTEXT_TAKEN();
		glGenQueries(n, queries);
		gGLLiveObjects->genQueries(n, queries);
	}

	void OpenGLDevice::createVertexArrays(GLsizei n, GLuint* arrays) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateVertexArrays(n, arrays);
		gGLLiveObjects->createVertexArrays(n, arrays);
	}

	void OpenGLDevice::createTextures(GLenum target, GLsizei n, GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateTextures(target, n, textures);
		gGLLiveObjects->createTextures(target, n, textures);
	}

	void OpenGLDevice::createFramebuffers(GLsizei n, GLuint* framebuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateFramebuffers(n, framebuffers);
		gGLLiveObjects->createFramebuffers(n, framebuffers);
	}

	void OpenGLDevice::createSamplers(GLsizei n, GLuint* samplers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateSamplers(n, samplers);
		gGLLiveObjects->createSamplers(n, samplers);
	}

	void OpenGLDevice::createQueries(GLenum target, GLsizei n, GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateQueries(target, n, ids);
		gGLLiveObjects->createQueries(target, n, ids);
	}

	void OpenGLDevice::createTransformFeedbacks(GLsizei n, GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateTransformFeedbacks(n, ids);
		gGLLiveObjects->createTransformFeedbacks(n, ids);
	}

	void OpenGLDevice::createBuffers(GLsizei n, GLuint* buffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateBuffers(n, buffers);
		gGLLiveObjects->createBuffers(n, buffers);
	}

	void OpenGLDevice::createRenderbuffers(GLsizei n, GLuint* renderbuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateRenderbuffers(n, renderbuffers);
		gGLLiveObjects->createRenderbuffers(n, renderbuffers);
	}

	void OpenGLDevice::createProgramPipelines(GLsizei n, GLuint* pipelines) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateProgramPipelines(n, pipelines);
		gGLLiveObjects->createProgramPipelines(n, pipelines);
	}

	// DELETE -----------------------------------------------------------------

	void OpenGLDevice::deleteVertexArrays(GLsizei n, const GLuint* arrays) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteVertexArrays(n, arrays);
		gGLLiveObjects->deleteVertexArrays(n, arrays);
	}

	void OpenGLDevice::deleteTextures(GLsizei n, const GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteTextures(n, textures);
		gGLLiveObjects->deleteTextures(n, textures);
	}

	void OpenGLDevice::deleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteFramebuffers(n, framebuffers);
		gGLLiveObjects->deleteFramebuffers(n, framebuffers);
	}

	void OpenGLDevice::deleteSamplers(GLsizei n, const GLuint* samplers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteSamplers(n, samplers);
		gGLLiveObjects->deleteSamplers(n, samplers);
	}

	void OpenGLDevice::deleteQueries(GLsizei n, const GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteQueries(n, ids);
		gGLLiveObjects->deleteQueries(n, ids);
	}

	void OpenGLDevice::deleteTransformFeedbacks(GLsizei n, const GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteTransformFeedbacks(n, ids);
		gGLLiveObjects->deleteTransformFeedbacks(n, ids);
	}

	void OpenGLDevice::deleteBuffers(GLsizei n, const GLuint* buffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteBuffers(n, buffers);
		gGLLiveObjects->deleteBuffers(n, buffers);
	}

	void OpenGLDevice::deleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteRenderbuffers(n, renderbuffers);
		gGLLiveObjects->deleteRenderbuffers(n, renderbuffers);
	}

	void OpenGLDevice::deleteProgramPipelines(GLsizei n, const GLuint* pipelines) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteProgramPipelines(n, pipelines);
		gGLLiveObjects->deleteProgramPipelines(n, pipelines);
	}

	// MISC -----------------------------------------------------------------

	void OpenGLDevice::objectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label) {
		CHECK_GL_CONTEXT_TAKEN();
		CHECKF(::strlen(label) <= capabilities.glObjectLabelMaxLength, "objectLabel is too long");
		glObjectLabel(identifier, name, length, label);
	}

}
