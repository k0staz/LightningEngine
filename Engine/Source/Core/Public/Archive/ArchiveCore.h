#pragma once
#include <cstddef>

namespace LE::Archive
{
struct Context;
struct ArchiveReader;
struct ArchiveWriter;

template <class T>
bool InvokeArchive(Context&, ArchiveWriter&, const T&) = delete;

template <class T>
bool InvokeArchive(Context&, ArchiveReader&, T&) = delete;

template <class T>
bool Serialize(Context&, ArchiveWriter&, const T&);
template <class T>
bool Deserialize(Context&, ArchiveReader&, T&);
}
