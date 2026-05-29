#pragma once

#include "AssetManager/AssetCore.h"
#include "AssetManager/AssetStorageFactory.h"

namespace LE
{
#define REGISTER_ASSET_TYPE(Type, TypeName) \
	template<> \
	struct AssetTypeRegistration<Type> \
	{\
	static constexpr std::string_view Value = TypeName; \
	};
}