#pragma once
#include "CoreDefinitions.h"
#include "Misc/Traits.h"
#include "Math/Math.h"

namespace LE
{
using AssetId = uint32;
using AssetTypeId = uint32;

template <typename Type>
struct AssetTraits : IdTraitsInterpreter<IdTraits<Type>>
{
	static constexpr uint64 PageSize = ASSET_SPARSE_PAGE;
};

inline constexpr NullId AssetIdNull{};
inline constexpr NullId AssetTypeIdNull{};

template <class AssetType>
struct AssetTypeRegistration;

template <typename AssetType>
struct AssetTypeIdGetter
{
	static constexpr std::string_view TypeName = AssetTypeRegistration<AssetType>::Value;
	static constexpr AssetTypeId Value = FNV1AHash(TypeName);
};

enum class AssetState : uint8
{
	Uninitialized,
	FailedLoad,
	Loading,
	Loaded
};

template <typename Type>
concept DerivedFromAsset = std::is_base_of_v<class Asset, Type>;
}
