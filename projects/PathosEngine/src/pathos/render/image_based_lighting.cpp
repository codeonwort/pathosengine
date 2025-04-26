#include "image_based_lighting.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/buffer.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/scene/reflection_probe_component.h"
#include "pathos/util/log.h"
#include "pathos/console.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_irradianceProbeAtlas_tileSize("r.irradianceProbeAtlas.tileSize", -1, "Override tile size (if positive)");
	static ConsoleVariable<int32> cvar_irradianceProbeAtlas_tileCountX("r.irradianceProbeAtlas.tileCountX", -1, "Override tile count in x-axis (if positive)");
	static ConsoleVariable<int32> cvar_irradianceProbeAtlas_tileCountY("r.irradianceProbeAtlas.tileCountY", -1, "Override tile count in y-axis (if positive)");

	LightProbeScene::LightProbeScene() {
		irradianceProbeAtlasDesc.tileSize = 6;
		irradianceProbeAtlasDesc.tileCountX = 128;
		irradianceProbeAtlasDesc.tileCountY = 128;
	}

	void LightProbeScene::initializeIrradianceProbeAtlasDesc(const IrradianceProbeAtlasDesc& desc) {
		irradianceProbeAtlasDesc = desc;
	}

	void LightProbeScene::createIrradianceProbeAtlas() {
		if (irradianceProbeAtlas == nullptr) {
			if (cvar_irradianceProbeAtlas_tileSize.getInt() > 0) irradianceProbeAtlasDesc.tileSize = cvar_irradianceProbeAtlas_tileSize.getInt();
			if (cvar_irradianceProbeAtlas_tileCountX.getInt() > 0) irradianceProbeAtlasDesc.tileCountX = cvar_irradianceProbeAtlas_tileCountX.getInt();
			if (cvar_irradianceProbeAtlas_tileCountY.getInt() > 0) irradianceProbeAtlasDesc.tileCountY = cvar_irradianceProbeAtlas_tileCountY.getInt();

			uint32 paddedSide = (irradianceProbeAtlasDesc.tileSize + 2);
			uint32 atlasWidth = paddedSide * irradianceProbeAtlasDesc.tileCountX;
			uint32 atlasHeight = paddedSide * irradianceProbeAtlasDesc.tileCountY;

			irradianceProbeAtlas = makeUnique<RenderTarget2D>();
			irradianceProbeAtlas->respecTexture(atlasWidth, atlasHeight, pathos::IRRADIANCE_PROBE_FORMAT, "Texture_IrradianceProbeAtlas");
			irradianceProbeAtlas->immediateUpdateResource();

			depthProbeAtlas = makeUnique<RenderTarget2D>();
			depthProbeAtlas->respecTexture(atlasWidth, atlasHeight, pathos::DEPTH_PROBE_FORMAT, "Texture_DepthProbeAtlas");
			depthProbeAtlas->immediateUpdateResource();
		}
		if (irradianceSHBuffer == nullptr) {
			const uint32 tileCountX = irradianceProbeAtlasDesc.tileCountX;
			const uint32 tileCountY = irradianceProbeAtlasDesc.tileCountY;
			const uint32 maxProbes = (tileCountX * tileCountY + 5) / 6;
			const uint32 stride = sizeof(float) * 4 * 9;

			BufferCreateParams desc{ EBufferUsage::CpuWrite, stride * maxProbes, nullptr, "Buffer_IrradianceSH" };
			irradianceSHBuffer = makeUnique<Buffer>(desc);
			irradianceSHBuffer->createGPUResource();
		}
	}

	IrradianceProbeID LightProbeScene::allocateIrradianceTiles(uint32 numRequiredTiles) {
		if (irradianceProbeAtlas == nullptr) {
			IrradianceProbeID probeID;
			probeID.firstTileID = IrradianceProbeAtlasDesc::INVALID_TILE_ID;
			probeID.firstShIndex = IrradianceProbeAtlasDesc::INVALID_TILE_ID;
			return probeID;
		}

		uint32 shIndex = 0;
		uint32 beginID = 0, endID = numRequiredTiles - 1;
		for (size_t i = 0; i < irradianceTileAllocs.size(); ++i) {
			const IrradianceTileRange& allocRange = irradianceTileAllocs[i];
			if (beginID <= allocRange.end && allocRange.begin <= endID) {
				beginID = allocRange.end + 1;
				endID = beginID + numRequiredTiles - 1;
			} else {
				break;
			}
		}
		if (endID < irradianceProbeAtlasDesc.totalTileCount()) {
			irradianceTileAllocs.push_back(IrradianceTileRange{ beginID, endID });
			IrradianceProbeID probeID;
			probeID.firstTileID = beginID;
			probeID.firstShIndex = beginID;
			return probeID;
		}

		// Failed to allocate tiles. Find out the reason.
		uint32 remainingTiles = irradianceProbeAtlasDesc.totalTileCount();
		for (const auto& allocRange : irradianceTileAllocs) {
			remainingTiles -= allocRange.end - allocRange.begin + 1;
		}
		if (remainingTiles < numRequiredTiles) {
			LOG(LogWarning, "%s: Overflow. Required: %u, remaining: %u", __FUNCTION__, numRequiredTiles, remainingTiles);
		} else {
			LOG(LogWarning, "%s: Fragmentation. Required: %u, remaining: %u but remaining tiles are not contiguous", __FUNCTION__, numRequiredTiles, remainingTiles);
		}
		IrradianceProbeID probeID;
		probeID.firstTileID = IrradianceProbeAtlasDesc::INVALID_TILE_ID;
		probeID.firstShIndex = IrradianceProbeAtlasDesc::INVALID_TILE_ID;
		return probeID;
	}

	bool LightProbeScene::freeIrradianceTiles(uint32 firstTileID, uint32 lastTileID) {
		auto it = std::find(irradianceTileAllocs.begin(), irradianceTileAllocs.end(), IrradianceTileRange{ firstTileID, lastTileID });
		if (it != irradianceTileAllocs.end()) {
			irradianceTileAllocs.erase(it);
			return true;
		}
		return false;
	}

	void LightProbeScene::getIrradianceTileTexelOffset(uint32 tileID, uint32& outX, uint32& outY) const {
		const auto& desc = irradianceProbeAtlasDesc;
		outX = 1 + (tileID % desc.tileCountX) * (2 + desc.tileSize);
		outY = 1 + (tileID / desc.tileCountX) * (2 + desc.tileSize);
	}

	void LightProbeScene::getIrradianceTileBounds(uint32 tileID, vector4& outBounds) const {
		if (irradianceProbeAtlas != nullptr) {
			uint32 x0, y0, x1, y1;
			getIrradianceTileTexelOffset(tileID, x0, y0);
			x1 = x0 + irradianceProbeAtlasDesc.tileSize;
			y1 = y0 + irradianceProbeAtlasDesc.tileSize;
			float dx = 0.5f / (float)irradianceProbeAtlas->getWidth();
			float dy = 0.5f / (float)irradianceProbeAtlas->getHeight();
			outBounds.x = +dx + x0 / (float)irradianceProbeAtlas->getWidth();
			outBounds.y = +dy + y0 / (float)irradianceProbeAtlas->getHeight();
			outBounds.z = -dx + x1 / (float)irradianceProbeAtlas->getWidth();
			outBounds.w = -dy + y1 / (float)irradianceProbeAtlas->getHeight();
		}
	}

	GLuint LightProbeScene::getIrradianceProbeAtlasTexture() const {
		return irradianceProbeAtlas->getInternalTexture()->internal_getGLName();
	}

	GLuint LightProbeScene::getDepthProbeAtlasTexture() const {
		return depthProbeAtlas->getInternalTexture()->internal_getGLName();
	}

	void LightProbeScene::createSceneProxy(SceneProxy* proxy, bool isLightProbeRendering) {
		if (irradianceProbeAtlas != nullptr) {
			proxy->irradianceAtlas       = irradianceProbeAtlas->getInternalTexture()->internal_getGLName();
			proxy->depthProbeAtlas       = depthProbeAtlas->getInternalTexture()->internal_getGLName();
			proxy->irradianceAtlasWidth  = (float)irradianceProbeAtlas->getWidth();
			proxy->irradianceAtlasHeight = (float)irradianceProbeAtlas->getHeight();
			proxy->irradianceTileCountX  = irradianceProbeAtlasDesc.tileCountX;
			proxy->irradianceTileSize    = irradianceProbeAtlasDesc.tileSize;
		}

		// Update irradiance volume buffer.
		if (isLightProbeRendering == false) {
			const size_t numVolumes = proxy->proxyList_irradianceVolume.size();
			size_t numPrevVolumes = 0;
			if (irradianceVolumeBuffer != nullptr) {
				numPrevVolumes = irradianceVolumeBuffer->getCreateParams().bufferSize / sizeof(IrradianceVolumeInfo);
			}
			if (numVolumes == 0) {
				if (irradianceVolumeBuffer != nullptr) {
					irradianceVolumeBuffer->releaseGPUResource();
					irradianceVolumeBuffer = nullptr;
				}
			} else {
				// Buffer size does not fit; recreate the buffer.
				if (numPrevVolumes < numVolumes || numPrevVolumes > numVolumes * 2) {
					if (irradianceVolumeBuffer != nullptr) {
						irradianceVolumeBuffer->releaseGPUResource();
					}
					BufferCreateParams createParams{
						EBufferUsage::CpuWrite,
						(uint32)(numVolumes * sizeof(IrradianceVolumeInfo)),
						nullptr,
						"Buffer_SSBO_IrradianceVolume",
					};
					irradianceVolumeBuffer = makeUnique<Buffer>(createParams);
					irradianceVolumeBuffer->createGPUResource();
				}
				// Upload the data.
				// #todo-light-probe: Reupload only if changed.
				std::vector<IrradianceVolumeInfo> bufferData;
				for (const IrradianceVolumeProxy* volumeProxy : proxy->proxyList_irradianceVolume) {
					IrradianceVolumeInfo bufferItem{
						volumeProxy->minBounds,
						volumeProxy->irradianceTileFirstID,
						volumeProxy->maxBounds,
						volumeProxy->numProbes,
						volumeProxy->gridSize,
						volumeProxy->captureRadius,
					};
					bufferData.emplace_back(bufferItem);
				}
				irradianceVolumeBuffer->writeToGPU(0, bufferData.size() * sizeof(IrradianceVolumeInfo), bufferData.data());
			}
		}
		proxy->irradianceVolumeBuffer = (irradianceVolumeBuffer != nullptr) ? irradianceVolumeBuffer->internal_getGLName() : 0;

		// Update reflection probe buffer.
		if (isLightProbeRendering == false) {
			const size_t numProbes = proxy->proxyList_reflectionProbe.size();
			size_t numPrevProbes = 0;
			if (reflectionProbeBuffer != nullptr) {
				numPrevProbes = reflectionProbeBuffer->getCreateParams().bufferSize / sizeof(ReflectionProbeInfo);
			}
			if (numProbes == 0) {
				if (reflectionProbeBuffer != nullptr) {
					reflectionProbeBuffer->releaseGPUResource();
					reflectionProbeBuffer = nullptr;
				}
			} else {
				// Buffer size does not fit; recreate the buffer.
				if (numPrevProbes < numProbes || numPrevProbes > numProbes * 2) {
					if (reflectionProbeBuffer != nullptr) {
						reflectionProbeBuffer->releaseGPUResource();
					}
					BufferCreateParams createParams{
						EBufferUsage::CpuWrite,
						(uint32)(numProbes * sizeof(ReflectionProbeInfo)),
						nullptr,
						"Buffer_SSBO_IrradianceVolume",
					};
					reflectionProbeBuffer = makeUnique<Buffer>(createParams);
					reflectionProbeBuffer->createGPUResource();
				}
				// Upload the data.
				// #todo-light-probe: Reupload only if changed.
				std::vector<ReflectionProbeInfo> bufferData;
				for (const ReflectionProbeProxy* probeProxy : proxy->proxyList_reflectionProbe) {
					if (probeProxy->specularIBL == nullptr) {
						continue;
					}
					ReflectionProbeInfo bufferItem{
						probeProxy->positionWS,
						probeProxy->captureRadius,
					};
					bufferData.emplace_back(bufferItem);
				}
				reflectionProbeBuffer->writeToGPU(0, bufferData.size() * sizeof(ReflectionProbeInfo), bufferData.data());
			}
		}
		proxy->reflectionProbeBuffer = (reflectionProbeBuffer != nullptr) ? reflectionProbeBuffer->internal_getGLName() : 0;
	}

}
