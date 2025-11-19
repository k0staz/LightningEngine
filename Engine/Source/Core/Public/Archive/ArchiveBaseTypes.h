#pragma once

#include "Math/Vector4.h"
#include "Misc/Paths.h"
#include "unordered_set"
#include "unordered_map"

namespace LE::Archive
{
#define RAISE_ERROR(ErrorDesc) \
	{ \
	Ctx.Error.Raised = true; \
	Ctx.Error.Desc = ErrorDesc; \
	return false; \
	}

#define TRY_WRITE_TYPE(Value, ErrorDesc) \
	if(!Writer.WriteTrivialType(Value)) \
	RAISE_ERROR(ErrorDesc)

#define TRY_WRITE_BYTES(Data, Size, ErrorDesc) \
	if (!Writer.WriteBytes(Data, Size)) \
	RAISE_ERROR(ErrorDesc)

#define TRY_READ_TYPE(Value, ErrorDesc) \
	if(!Reader.ReadTrivialType(Value)) \
	RAISE_ERROR(ErrorDesc)

#define TRY_READ_BYTES(Data, Size, ErrorDesc) \
	if (!Reader.ReadBytes(Data, Size)) \
	RAISE_ERROR(ErrorDesc)

#define TRY_SERIALIZE(Data, ErrorDesc) \
	if (!Serialize(Ctx, Writer, Data)) \
	RAISE_ERROR(ErrorDesc)

#define TRY_DESERIALIZE(Data, ErrorDesc) \
	if (!Deserialize(Ctx, Reader, Data)) \
	RAISE_ERROR(ErrorDesc)

// Bool ------------------------------------------------------------------------------------
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const bool& Value)
{
	uint8 x = Value ? 1 : 0;
	TRY_WRITE_TYPE(x, "Failed writing bool")

	return true;
}

inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, bool& Value)
{
	uint8 x = 0;
	TRY_READ_TYPE(x, "Failed reading bool")

	Value = x != 0;
	return true;
}

// Arithmetic ------------------------------------------------------------------------------------
template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const T& Value)
{
	TRY_WRITE_TYPE(Value, "Failed writing arithmetic")

	return true;
}

template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, T& Value)
{
	TRY_READ_TYPE(Value, "Failed reading arithmetic")

	return true;
}

// Vector2<T> ------------------------------------------------------------------------------------
template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const Vector2<T>& Value)
{
	TRY_WRITE_TYPE(Value.X, "Failed writing arithmetic in Vector2")
	TRY_WRITE_TYPE(Value.Y, "Failed writing arithmetic in Vector2")

	return true;
}

template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, Vector2<T>& Value)
{
	TRY_READ_TYPE(Value.X, "Failed writing arithmetic in Vector2")
	TRY_READ_TYPE(Value.Y, "Failed writing arithmetic in Vector2")

	return true;
}

// Vector3<T> ------------------------------------------------------------------------------------
template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const Vector3<T>& Value)
{
	TRY_WRITE_TYPE(Value.X, "Failed writing arithmetic in Vector3")
	TRY_WRITE_TYPE(Value.Y, "Failed writing arithmetic in Vector3")
	TRY_WRITE_TYPE(Value.Z, "Failed writing arithmetic in Vector3")

	return true;
}

template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, Vector3<T>& Value)
{
	TRY_READ_TYPE(Value.X, "Failed writing arithmetic in Vector3")
	TRY_READ_TYPE(Value.Y, "Failed writing arithmetic in Vector3")
	TRY_READ_TYPE(Value.Z, "Failed writing arithmetic in Vector3")

	return true;
}

// Vector4<T> ------------------------------------------------------------------------------------
template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const Vector4<T>& Value)
{
	TRY_WRITE_TYPE(Value.X, "Failed writing arithmetic in Vector4")
	TRY_WRITE_TYPE(Value.Y, "Failed writing arithmetic in Vector4")
	TRY_WRITE_TYPE(Value.Z, "Failed writing arithmetic in Vector4")
	TRY_WRITE_TYPE(Value.W, "Failed writing arithmetic in Vector4")

	return true;
}

template <class T> requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, Vector4<T>& Value)
{
	TRY_READ_TYPE(Value.X, "Failed writing arithmetic in Vector4")
	TRY_READ_TYPE(Value.Y, "Failed writing arithmetic in Vector4")
	TRY_READ_TYPE(Value.Z, "Failed writing arithmetic in Vector4")
	TRY_READ_TYPE(Value.W, "Failed writing arithmetic in Vector4")

	return true;
}

// Enum ------------------------------------------------------------------------------------
template <class T> requires (std::is_enum_v<T>)
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const T& Value)
{
	using Underlying = std::underlying_type_t<T>;
	TRY_WRITE_TYPE(static_cast<const Underlying>(Value), "Failed writing enum")

	return true;
}

template <class T> requires (std::is_enum_v<T>)
inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, T& Value)
{
	using Underlying = std::underlying_type_t<T>;
	Underlying uValue;
	TRY_READ_TYPE(uValue, "Failed reading enum")

	Value = static_cast<T>(uValue);
	return true;
}

// std::string ------------------------------------------------------------------------------------
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const std::string& Value)
{
	// Write length
	const uint32 length = static_cast<uint32>(Value.size());
	TRY_WRITE_TYPE(length, "Failed writing string size")

	// Write string
	TRY_WRITE_BYTES(Value.data(), length, "Failed writing string")

	return true;
}

inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, std::string& Value)
{
	// Read length
	uint32 length = 0;
	TRY_READ_TYPE(length, "Failed reading string size")

	Value.resize(length);
	TRY_READ_BYTES(Value.data(), length, "Failed reading string")

	return true;
}

// byte blob std::vector<std::byte> ------------------------------------------------------------------------------------
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const std::vector<std::byte>& Value)
{
	// Write length
	const uint32 length = static_cast<uint32>(Value.size());
	TRY_WRITE_TYPE(length, "Failed writing byte blob size")

	// Write blob
	TRY_WRITE_BYTES(Value.data(), length, "Failed writing byte blob")

	return true;
}

inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, std::vector<std::byte>& Value)
{
	// Read length
	uint32 length = 0;
	TRY_READ_TYPE(length, "Failed reading byte blob size")

	Value.resize(length);
	TRY_READ_BYTES(Value.data(), length, "Failed reading byte blob")

	return true;
}

// std::vector<T> ------------------------------------------------------------------------------------
template <class T>
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const std::vector<T>& Value)
{
	// Write length
	const uint32 length = static_cast<uint32>(Value.size());
	TRY_WRITE_TYPE(length, "Failed writing byte blob size")

	// Write elements
	for (const T& element : Value)
	{
		TRY_SERIALIZE(element, "Failed writing vector element")
	}

	return true;
}

template <class T>
inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, std::vector<T>& Value)
{
	// Read length
	uint32 length = 0;
	TRY_READ_TYPE(length, "Failed reading byte blob size")

	Value.resize(length);
	for (T& element : Value)
	{
		TRY_DESERIALIZE(element, "Failed reading vector element")
	}

	return true;
}

// std::array<T, N> ------------------------------------------------------------------------------------
template <class T, size_t N>
bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const std::array<T, N>& Value)
{
	for (const T& element : Value)
	{
		TRY_SERIALIZE(element, "Failed writing array element")
	}

	return true;
}

template <class T, size_t N>
bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, std::array<T, N>& Value)
{
	for (T& element : Value)
	{
		TRY_DESERIALIZE(element, "Failed reading array element")
	}

	return true;
}

// span of bytes ------------------------------------------------------------------------------------
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const std::span<std::byte>& Value)
{
	TRY_WRITE_BYTES(Value.data(), Value.size(), "Failed writing span of bytes")

	return true;
}

inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, std::span<std::byte>& Value)
{
	TRY_READ_BYTES(Value.data(), Value.size(), "Failed reading span of bytes")

	return true;
}

// std::unordered_map<Key, Data>  ------------------------------------------------------------------------------------
template <class Key, class Data>
bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const std::unordered_map<Key, Data>& Value)
{
	uint64 count = static_cast<uint64>(Value.size());
	TRY_WRITE_TYPE(count, "Failed writing unordered map size")
	for (const auto& it : Value)
	{
		TRY_SERIALIZE(it.first, "Failed writing unordered map Key")
		TRY_SERIALIZE(it.second, "Failed writing unordered map Data")
	}

	return true;
}

template <class Key, class Data>
bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, std::unordered_map<Key, Data>& Value)
{
	uint64 count = 0;
	TRY_READ_TYPE(count, "Failed reading unordered map size")
	for (uint64 i = 0; i < count; ++i)
	{
		Key _key;
		TRY_DESERIALIZE(_key, "Failed reading unordered map Key")
		Data& _data = Value[_key];
		TRY_DESERIALIZE(_data, "Failed reading unordered map Key")
	}

	return true;
}

// Path ------------------------------------------------------------------------------------
inline bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const Path& Value)
{
	const std::string pathString = Value.generic_string();

	TRY_SERIALIZE(pathString, "Failed writing path as a string")

	return true;
}

inline bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, Path& Value)
{
	std::string pathString;

	TRY_DESERIALIZE(pathString, "Failed reading path as a string")

	Value = pathString;

	return true;
}

// std::unordered_set<Key>  ------------------------------------------------------------------------------------
template <class Key>
bool InvokeArchive(Context& Ctx, ArchiveWriter& Writer, const std::unordered_set<Key>& Value)
{
	uint64 count = static_cast<uint64>(Value.size());
	TRY_WRITE_TYPE(count, "Failed writing unordered map size")
	for (const Key& it : Value)
	{
		TRY_SERIALIZE(it, "Failed writing unordered set element")
	}

	return true;
}

template <class Key>
bool InvokeArchive(Context& Ctx, ArchiveReader& Reader, std::unordered_set<Key>& Value)
{
	uint64 count = 0;
	TRY_READ_TYPE(count, "Failed reading unordered map size")
	for (uint64 i = 0; i < count; ++i)
	{
		Key _key;
		TRY_DESERIALIZE(_key, "Failed reading unordered map Key")
		Value.emplace(_key);
	}

	return true;
}
}
