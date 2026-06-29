#pragma once

#include "AssetManager/AssetManager.h"
#include "Assets/TextureAsset.h"

namespace LE
{
class MaterialInstanceAsset : public Asset
{
public:
    using Asset::Asset;

    AssetHandle<TextureAsset> BaseColorTexture = {};
    AssetHandle<TextureAsset> NormalTexture = {};
    AssetHandle<TextureAsset> HeightTexture = {};
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const MaterialInstanceAsset& Value)
{
    using namespace LE::Archive;

    if (!Serialize(Ctx, Writer, static_cast<const Asset&>(Value)))
    {
        return false;
    }

    if (!Serialize(Ctx, Writer, Value.BaseColorTexture))
    {
        return false;
    }

    if (!Serialize(Ctx, Writer, Value.NormalTexture))
    {
        return false;
    }

    if (!Serialize(Ctx, Writer, Value.HeightTexture))
    {
        return false;
    }

    return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, MaterialInstanceAsset& Value)
{
    using namespace LE::Archive;

    if (!Deserialize(Ctx, Reader, static_cast<Asset&>(Value)))
    {
        return false;
    }

    if (!Deserialize(Ctx, Reader, Value.BaseColorTexture))
    {
        return false;
    }

    if (!Deserialize(Ctx, Reader, Value.NormalTexture))
    {
        return false;
    }

    if (!Deserialize(Ctx, Reader, Value.HeightTexture))
    {
        return false;
    }

    return true;
}

REGISTER_ASSET_TYPE(MaterialInstanceAsset, "MaterialInstanceAsset")
}
