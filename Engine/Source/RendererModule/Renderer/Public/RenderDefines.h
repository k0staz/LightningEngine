#pragma once
#include "CoreDefinitions.h"

namespace LE::Renderer
{
enum class RenderPassType : uint8
{
	Base,

	Count,
};

constexpr uint64 DEFAULT_MESH_GLOBAL_BUFFER_SIZE = 256 * 1024 * 1024;
inline uint64 GlobalMeshGlobalBufferSize   = DEFAULT_MESH_GLOBAL_BUFFER_SIZE;

inline uint64 GlobalMeshPositionsSize = (GlobalMeshGlobalBufferSize * 40) / 100;
inline uint64 GlobalMeshIndicesSize   = (GlobalMeshGlobalBufferSize * 25) / 100;
inline uint64 GlobalMeshNormalsSize   = (GlobalMeshGlobalBufferSize * 20) / 100;
inline uint64 GlobalMeshTexCoordsSize = (GlobalMeshGlobalBufferSize * 14) / 100;

constexpr uint64 DEFAULT_FRAME_DATA_BUFFER_SIZE = 16 * 1024 * 1024;
inline uint64 GlobalFrameDataBufferSize   = DEFAULT_FRAME_DATA_BUFFER_SIZE;
}
