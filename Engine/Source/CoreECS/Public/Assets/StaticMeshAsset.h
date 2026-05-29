#pragma once
#include "StaticMesh/StaticMeshRendering.h"
#include "AssetManager/Asset.h"
#include "AssetManager/AssetManager.h"
#include "AssetManager/AssetRegistrationUtil.h"

namespace LE
{
class StaticMeshAsset : public Asset
{
public:
	using Asset::Asset;

	std::vector<uint16> Indices;
	std::vector<Renderer::StaticMeshVertex> Vertices;
	RHI::PrimitiveType PrimitiveType = RHI::PrimitiveType::TriangleList;
};

REGISTER_ASSET_TYPE(StaticMeshAsset, "StaticMeshAsset")

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const StaticMeshAsset& Value)
{
	using namespace LE::Archive;

	if (!Serialize(Ctx, Writer, static_cast<const Asset&>(Value)))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.Indices))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.Vertices))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.PrimitiveType))
	{
		return false;
	}

	return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, StaticMeshAsset& Value)
{
	using namespace LE::Archive;

	if (!Deserialize(Ctx, Reader, static_cast<Asset&>(Value)))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.Indices))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.Vertices))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.PrimitiveType))
	{
		return false;
	}

	return true;
}

// TODO: Remove once done testing
class TestAsset : public Asset
{
public:
	using Asset::Asset;

	AssetHandle<StaticMeshAsset> StaticMesh;
	AssetHandle<StaticMeshAsset> StaticMesh2;
};

REGISTER_ASSET_TYPE(TestAsset, "TestAsset")

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const TestAsset& Value)
{
	using namespace LE::Archive;

	if (!Serialize(Ctx, Writer, static_cast<const Asset&>(Value)))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.StaticMesh))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.StaticMesh2))
	{
		return false;
	}
	
	return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, TestAsset& Value)
{
	using namespace LE::Archive;

	if (!Deserialize(Ctx, Reader, static_cast<Asset&>(Value)))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.StaticMesh))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.StaticMesh2))
	{
		return false;
	}
	
	return true;
}
}
