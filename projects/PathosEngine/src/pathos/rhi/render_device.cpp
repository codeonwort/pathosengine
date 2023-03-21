#include "render_device.h"
#include "pathos/console.h"
#include "pathos/util/log.h"
#include "pathos/rhi/gl_context_manager.h"
#include "pathos/rhi/shader_program.h"

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

// Do I have to do this manually?
// https://www.khronos.org/opengl/wiki/Texture_Storage
static uint32 getBytesOfInternalformat(GLint internalformat) {
	switch (internalformat) {
		case GL_RGBA32F: case GL_RGBA32UI: case GL_RGBA32I:
			return 16;
		case GL_RGB32F: case GL_RGB32UI: case GL_RGB32I:
			return 12;
		case GL_RGBA16F: case GL_RG32F: case GL_RGBA16UI:
		case GL_RG32UI: case GL_RGBA16I: case GL_RG32I:
		case GL_RGBA16: case GL_RGBA16_SNORM:
			return 8;
		case GL_RGB16: case GL_RGB16_SNORM: case GL_RGB16F:
		case GL_RGB16UI: case GL_RGB16I:
			return 6;
		case GL_RG16F: case GL_R11F_G11F_B10F: case GL_R32F:
		case GL_RGB10_A2UI: case GL_RGBA8UI: case GL_RG16UI:
		case GL_R32UI: case GL_RGBA8I: case GL_RG16I:
		case GL_R32I: case GL_RGB10_A2: case GL_RGBA8: case GL_RG16:
		case GL_RGBA8_SNORM: case GL_RG16_SNORM: case GL_SRGB8_ALPHA8: case GL_RGB9_E5:
			return 4;
		case GL_RGB8: case GL_RGB8_SNORM: case GL_SRGB8: case GL_RGB8UI: case GL_RGB8I:
			return 3;
		case GL_R16F: case GL_RG8UI: case GL_R16UI: case GL_RG8I:
		case GL_R16I: case GL_RG8: case GL_R16: case GL_RG8_SNORM: case GL_R16_SNORM:
			return 2;
		case GL_R8UI: case GL_R8I: case GL_R8: case GL_R8_SNORM:
			return 1;
	}
	// #todo: Compressed formats
	return 0;
}

namespace pathos {

	OpenGLDevice* gRenderDevice = nullptr;

	// NOTE: Should be set in EngineConfig.ini to be effective.
	static ConsoleVariable<int32> cvarDumpGLDevice("r.dumpGLDevice", 0, "(read only) Dump GL device info to log/device_dump");

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

		// #todo-renderthread: Is it safe to pass flushCondVar like this?
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

		// #todo-renderthread: Is it safe to pass flushCondVar like this?
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

		::memset(&extensionSupport, 0, sizeof(extensionSupport));
	}

	OpenGLDevice::~OpenGLDevice() {
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
			// #todo: Support AMD driver
			LOG(LogInfo, "[RenderDevice] VRAM: unknown ('NVX_gpu_memory_info' extension is missing)");
		}

		return true;
	}

	void OpenGLDevice::memreport(int64& outTotalBufferMemory, int64& outTotalTextureMemory) {
		outTotalBufferMemory = 0;
		outTotalTextureMemory = 0;
		for (GLuint buffer : aliveGLBuffers) {
			int64 bufferSize = 0;
			glGetNamedBufferParameteri64v(buffer, GL_BUFFER_SIZE, &bufferSize);
			outTotalBufferMemory += bufferSize;
		}
		for (GLuint texture : aliveGLTextures) {
			if (glIsTexture(texture) == false) {
				// How is this a case
				continue;
			}
			GLint baseMip, maxMip;
#if 0
			glGetTextureParameteriv(texture, GL_TEXTURE_BASE_LEVEL, &baseMip);
			glGetTextureParameteriv(texture, GL_TEXTURE_MAX_LEVEL, &maxMip);
#else
			// Querying GL_TEXTURE_MAX_LEVEL always returns 1000 by default :/
			// But I'm not tracking mip levels when calling glTextureStorageXXX().
			// Let's just consider first mip.
			baseMip = maxMip = 0;
#endif
			for (GLint mip = baseMip; mip <= maxMip; ++mip) {
				GLint width, height, depth, internalformat;
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_WIDTH, &width);
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_HEIGHT, &height);
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_DEPTH, &depth);
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
				uint32 bytesPerPixel = getBytesOfInternalformat(internalformat);
				int64 mipSize = width * height * depth * bytesPerPixel;
				outTotalTextureMemory += mipSize;
			}
		}
	}

	// Cannot track GL objects that were created by direct GL calls.
	void OpenGLDevice::reportLiveObjects() {
		LOG(LogDebug, "=== Report Live Objects ===");
		LOG(LogDebug, "Vertex Array       : %u", aliveGLVertexArrays.size());
		LOG(LogDebug, "Texture            : %u", aliveGLTextures.size());
		LOG(LogDebug, "Framebuffer        : %u", aliveGLFramebuffers.size());
		LOG(LogDebug, "Sampler            : %u", aliveGLSamplers.size());
		LOG(LogDebug, "Query              : %u", aliveGLQueries.size());
		LOG(LogDebug, "Transform Feedback : %u", aliveGLTransformFeedbacks.size());
		LOG(LogDebug, "Buffer             : %u", aliveGLBuffers.size());
		LOG(LogDebug, "Renderbuffer       : %u", aliveGLRenderBuffers.size());
		LOG(LogDebug, "Program Pipeline   : %u", aliveGLProgramPipelines.size());
		LOG(LogDebug, "Program            : %u", ShaderDB::get().programMap.size());
		
		size_t totalAlive = 0;
		totalAlive += aliveGLVertexArrays.size();
		totalAlive += aliveGLTextures.size();
		totalAlive += aliveGLFramebuffers.size();
		totalAlive += aliveGLSamplers.size();
		totalAlive += aliveGLQueries.size();
		totalAlive += aliveGLTransformFeedbacks.size();
		totalAlive += aliveGLBuffers.size();
		totalAlive += aliveGLRenderBuffers.size();
		totalAlive += aliveGLProgramPipelines.size();
		totalAlive += ShaderDB::get().programMap.size();
		LOG(LogDebug, "> Total %u GL objects are leaking", totalAlive);
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

		// https://registry.khronos.org/OpenGL/extensions/NVX/NVX_gpu_memory_info.txt
		if (extensionSupport.NVX_gpu_memory_info != 0) {
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
		extensionSupport.NV_ray_tracing                  = findExt("GL_NV_ray_tracing");
		extensionSupport.NV_mesh_shader                  = findExt("GL_NV_mesh_shader");
		extensionSupport.NV_shading_rate_image           = findExt("GL_NV_shading_rate_image");
		extensionSupport.NV_shader_texture_footprint     = findExt("GL_NV_shader_texture_footprint");
		extensionSupport.NV_representative_fragment_test = findExt("GL_NV_representative_fragment_test");
		extensionSupport.NV_fragment_shader_barycentric  = findExt("GL_NV_fragment_shader_barycentric");
		extensionSupport.NV_compute_shader_derivatives   = findExt("GL_NV_compute_shader_derivatives");
		extensionSupport.NV_scissor_exclusive            = findExt("GL_NV_scissor_exclusive");
		extensionSupport.NVX_gpu_memory_info             = findExt("GL_NVX_gpu_memory_info");
		extensionSupport.NV_gpu_shader5                  = findExt("GL_NV_gpu_shader5");
		extensionSupport.EXT_shader_16bit_storage        = findExt("GL_EXT_shader_16bit_storage");
		extensionSupport.EXT_shader_explicit_arithmetic_types = findExt("GL_EXT_shader_explicit_arithmetic_types");
	}

}

// API for GPU resource creation and deletion. (not queued in command list)
// 
namespace pathos {

	// CREATE -----------------------------------------------------------------

	void OpenGLDevice::genTextures(GLsizei n, GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glGenTextures(n, textures);
		for (GLsizei i = 0; i < n; ++i) aliveGLTextures.insert(textures[i]);
	}

	void OpenGLDevice::genQueries(GLsizei n, GLuint* queries) {
		CHECK_GL_CONTEXT_TAKEN();
		glGenQueries(n, queries);
		for (GLsizei i = 0; i < n; ++i) aliveGLQueries.insert(queries[i]);
	}

	void OpenGLDevice::createTextures(GLenum target, GLsizei n, GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateTextures(target, n, textures);
		for (GLsizei i = 0; i < n; ++i) aliveGLTextures.insert(textures[i]);
	}

	void OpenGLDevice::createFramebuffers(GLsizei n, GLuint* framebuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateFramebuffers(n, framebuffers);
		for (GLsizei i = 0; i < n; ++i) aliveGLFramebuffers.insert(framebuffers[i]);
	}

	void OpenGLDevice::createVertexArrays(GLsizei n, GLuint* arrays) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateVertexArrays(n, arrays);
		for (GLsizei i = 0; i < n; ++i) aliveGLVertexArrays.insert(arrays[i]);
	}

	void OpenGLDevice::createSamplers(GLsizei n, GLuint* samplers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateSamplers(n, samplers);
		for (GLsizei i = 0; i < n; ++i) aliveGLSamplers.insert(samplers[i]);
	}

	void OpenGLDevice::createQueries(GLenum target, GLsizei n, GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateQueries(target, n, ids);
		for (GLsizei i = 0; i < n; ++i) aliveGLQueries.insert(ids[i]);
	}

	void OpenGLDevice::createTransformFeedbacks(GLsizei n, GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateTransformFeedbacks(n, ids);
		for (GLsizei i = 0; i < n; ++i) aliveGLTransformFeedbacks.insert(ids[i]);
	}

	void OpenGLDevice::createBuffers(GLsizei n, GLuint* buffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateBuffers(n, buffers);
		for (GLsizei i = 0; i < n; ++i) aliveGLBuffers.insert(buffers[i]);
	}

	void OpenGLDevice::createRenderbuffers(GLsizei n, GLuint* renderbuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateRenderbuffers(n, renderbuffers);
		for (GLsizei i = 0; i < n; ++i) aliveGLRenderBuffers.insert(renderbuffers[i]);
	}

	void OpenGLDevice::createProgramPipelines(GLsizei n, GLuint* pipelines) {
		CHECK_GL_CONTEXT_TAKEN();
		glCreateProgramPipelines(n, pipelines);
		for (GLsizei i = 0; i < n; ++i) aliveGLProgramPipelines.insert(pipelines[i]);
	}

	// DELETE -----------------------------------------------------------------

	void OpenGLDevice::deleteQueries(GLsizei n, const GLuint* ids) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteQueries(n, ids);
		for (GLsizei i = 0; i < n; ++i) aliveGLQueries.erase(ids[i]);
	}

	void OpenGLDevice::deleteBuffers(GLsizei n, const GLuint* buffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteBuffers(n, buffers);
		for (GLsizei i = 0; i < n; ++i) aliveGLBuffers.erase(buffers[i]);
	}

	void OpenGLDevice::deleteProgramPipelines(GLsizei n, GLuint* pipelines)
	{
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteProgramPipelines(n, pipelines);
		for (GLsizei i = 0; i < n; ++i) aliveGLProgramPipelines.erase(pipelines[i]);
	}

	void OpenGLDevice::deleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteFramebuffers(n, framebuffers);
		for (GLsizei i = 0; i < n; ++i) aliveGLFramebuffers.erase(framebuffers[i]);
	}

	void OpenGLDevice::deleteSamplers(GLsizei n, const GLuint* samplers) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteSamplers(n, samplers);
		for (GLsizei i = 0; i < n; ++i) aliveGLSamplers.erase(samplers[i]);
	}

	void OpenGLDevice::deleteVertexArrays(GLsizei n, const GLuint* arrays) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteVertexArrays(n, arrays);
		for (GLsizei i = 0; i < n; ++i) aliveGLVertexArrays.erase(arrays[i]);
	}

	void OpenGLDevice::deleteTextures(GLsizei n, const GLuint* textures) {
		CHECK_GL_CONTEXT_TAKEN();
		glDeleteTextures(n, textures);
		for (GLsizei i = 0; i < n; ++i) aliveGLTextures.erase(textures[i]);
	}

	// MISC -----------------------------------------------------------------

	void OpenGLDevice::objectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label) {
		CHECK_GL_CONTEXT_TAKEN();
		CHECKF(::strlen(label) <= capabilities.glObjectLabelMaxLength, "objectLabel is too long");
		glObjectLabel(identifier, name, length, label);
	}

}
