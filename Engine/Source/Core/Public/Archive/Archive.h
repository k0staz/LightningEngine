#pragma once

#include "ArchiveCore.h"
#include "ArchiveIO.h"
#include "ArchiveBaseTypes.h"

namespace LE::Archive
{
template <class T>
bool Serialize(Context& Ctx, ArchiveWriter& Ar, const T& Value)
{
	return InvokeArchive(Ctx, Ar, Value);
}

template <class T>
bool Deserialize(Context& Ctx, ArchiveReader& Ar, T& Value)
{
	return InvokeArchive(Ctx, Ar, Value);
}
}
