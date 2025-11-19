#pragma once
#include <array>
#include <random>
#include <functional>

#include "CoreDefinitions.h"
#include "Math/Math.h"
#include "Archive/Archive.h"

namespace LE
{
struct Uid
{
	using DataType = std::array<uint8, 16>;
	Uid() = default;

	bool IsValid() const
	{
		for (uint8 byte : Data)
		{
			if (byte != 0)
			{
				return true;
			}
		}

		return false;
	}

	static Uid GenerateUid()
	{
		std::random_device rd;
		std::mt19937 twisterEngine(rd());
		std::uniform_int_distribution<uint64> distribution;

		DataType data;
		uint64 r0 = distribution(twisterEngine);
		uint64 r1 = distribution(twisterEngine);
		std::memcpy(data.data(), &r0, sizeof(uint64));
		std::memcpy(data.data() + 8, &r1, sizeof(uint64));

		// Set version (4)
		data[6] = (data[6] & 0x0F) | 0x40;
		// Set variant (RFC 4122)
		data[8] = (data[8] & 0x3F) | 0x80;

		return {data};
	}

	constexpr static Uid FromString(const std::string_view Str)
	{
		DataType data;

		if (Str.empty())
		{
			return {data};
		}

		const bool hasBrackets = Str.front() == '{';
		if (hasBrackets && Str.back() != '}')
		{
			return {data};
		}

		size_t index = 0;
		bool isHigh = true;
		for (size_t i = hasBrackets; i < Str.size() - hasBrackets; ++i)
		{
			if (Str[i] == '-')
			{
				continue;
			}

			if (index >= 16 || !IsHex(Str[i]))
			{
				return {data};
			}

			if (isHigh)
			{
				data[index] = static_cast<uint8>(HexToChar(Str[i]) << 4);
				isHigh = false;
			}
			else
			{
				data[index] = static_cast<uint8>(data[index] | HexToChar(Str[i]));
				++index;
				isHigh = true;
			}
		}

		return {data};
	}

	static constexpr char EmptyUidString[] = "00000000-0000-0000-0000-000000000000";

	static std::string ToString(const Uid& Id)
	{
		std::string str{EmptyUidString};
		constexpr char encoder[17] = "0123456789abcdef";

		for (size_t i = 0, index = 0; i < 36; ++i)
		{
			if (i == 8 || i == 13 || i == 18 || i == 23)
			{
				continue;
			}
			str[i] = encoder[Id.Data[index] >> 4 & 0x0f];
			str[++i] = encoder[Id.Data[index] & 0x0f];
			index++;
		}

		return str;
	}

	friend std::ostream& operator<<(std::ostream& OStream, const Uid& Id)
	{
		OStream << ToString(Id);
		return OStream;
	}

	static uint64 SplitMix64Hash(const Uid& Id)
	{
		uint64 a = 0;
		std::memcpy(&a, Id.Data.data(), sizeof(uint64));
		uint64 b = 0;
		std::memcpy(&b, Id.Data.data() + 8, sizeof(uint64));

		auto sm64 = [](uint64 x)
		{
			x += 0x9e3779b97f4a7c15ull;
			x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
			x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
			return x ^ (x >> 31);
		};

		return sm64(a) ^ sm64(b);
	}

private:
	Uid(const DataType& InputData)
		: Data(InputData)
	{
	}

	DataType Data = {};

	friend bool operator==(Uid const& Lhs, Uid const& Rhs) noexcept;
	friend bool operator<(Uid const& Lhs, Uid const& Rhs) noexcept;

	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const Uid& Value);
	friend bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, Uid& Value);
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const Uid& Value)
{
	if (!Archive::Serialize(Ctx, Writer, Value.Data))
	{
		return false;
	}

	return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, Uid& Value)
{
	if (!Archive::Deserialize(Ctx, Reader, Value.Data))
	{
		return false;
	}

	return true;
}

inline constexpr Uid EmptyUid = {};

inline bool operator==(Uid const& Lhs, Uid const& Rhs) noexcept
{
	return Lhs.Data == Rhs.Data;
}

inline bool operator!=(Uid const& Lhs, Uid const& Rhs) noexcept
{
	return !(Lhs == Rhs);
}

inline bool operator<(Uid const& Lhs, Uid const& Rhs) noexcept
{
	return Lhs.Data < Rhs.Data;
}
}

namespace std
{
template <>
struct hash<LE::Uid>
{
	size_t operator()(const LE::Uid& obj) const
	{
		return LE::Uid::SplitMix64Hash(obj);
	}
};
}
