#pragma once
#include "AssetManager/Asset.h"
#include "RHIDefinitions.h"
#include "AssetManager/AssetRegistrationUtil.h"

namespace LE
{
class StaticMeshAsset : public Asset
{
public:
	using Asset::Asset;

	std::vector<uint32> Indices;
	std::vector<Vector3F> Positions;
	std::vector<Vector3F> Normals;
	std::vector<Vector2F> UVs;

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

	if (!Serialize(Ctx, Writer, Value.Positions))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.Normals))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.UVs))
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

	if (!Deserialize(Ctx, Reader, Value.Positions))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.Normals))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.UVs))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.PrimitiveType))
	{
		return false;
	}

	return true;
}
}
