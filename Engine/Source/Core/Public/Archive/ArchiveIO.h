#pragma once

#include "ArchiveCore.h"
#include <string>
#include <span>
#include <type_traits>

#include "CoreDefinitions.h"

namespace LE
{
struct AssetInfo;
}

namespace LE::Archive
{
struct Context
{
	Context(AssetInfo* InInfo)
		: Info(InInfo)
	{
	}

	struct ArchiveError
	{
		bool Raised = false;
		std::string Desc;
	};

	ArchiveError Error;
	AssetInfo* Info = nullptr;
};

struct ArchiveReader
{
	ArchiveReader(std::span<const std::byte> InBuffer)
		: Buffer(InBuffer)
	{
	}

	bool ReadBytes(void* Dest, size_t Size)
	{
		if (ReadOffset + Size > Buffer.size())
		{
			return false;
		}

		std::memcpy(Dest, Buffer.data() + ReadOffset, Size);
		ReadOffset += Size;
		return true;
	}

	template <class T>
	bool ReadTrivialType(T& Value)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Wrong type");
		return ReadBytes(&Value, sizeof(T));
	}

	void Skip(size_t Size)
	{
		ReadOffset += Size;
	}

private:
	std::span<const std::byte> Buffer;
	size_t ReadOffset = 0;
};

struct ArchiveWriter
{
	ArchiveWriter(std::vector<std::byte>& Buffer)
		: OutBuffer(Buffer)
	{
	}

	bool WriteBytes(const void* Source, size_t Size)
	{
		const size_t writeOffset = OutBuffer.size();
		OutBuffer.resize(writeOffset + Size);
		std::memcpy(OutBuffer.data() + writeOffset, Source, Size);

		return true;
	}

	template <class T>
	bool WriteTrivialType(const T& Value)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Wrong type");
		return WriteBytes(&Value, sizeof(T));
	}

private:
	std::vector<std::byte>& OutBuffer;
};
}
