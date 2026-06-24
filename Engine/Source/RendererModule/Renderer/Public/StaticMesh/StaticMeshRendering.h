#pragma once

#include "Archive/Archive.h"
#include "Math/Vector3.h"

namespace LE::Renderer
{
struct StaticMeshVertex
{
	Vector3F Position;

	Vector3F Normal;

	Vector2F TextureCord;
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const StaticMeshVertex& Value)
{
	if (!Serialize(Ctx, Writer, Value.Position))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.Normal))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.TextureCord))
	{
		return false;
	}

	return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, StaticMeshVertex& Value)
{
	if (!Deserialize(Ctx, Reader, Value.Position))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.Normal))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.TextureCord))
	{
		return false;
	}

	return true;
}
}