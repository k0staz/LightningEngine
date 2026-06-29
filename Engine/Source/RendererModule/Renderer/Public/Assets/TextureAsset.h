#pragma once
#include "AssetManager/Asset.h"
#include "AssetManager/AssetRegistrationUtil.h"

#include <ktx.h>

namespace LE
{
class TextureAsset : public Asset
{
public:
    using Asset::Asset;

    ~TextureAsset() override
    {
        if (KtxTexture)
        {
            ktxTexture2_Destroy(KtxTexture);
        }
    }

    TextureAsset(TextureAsset&& other) noexcept
        : Asset(std::move(other))
    {
        KtxTexture = other.KtxTexture;
        other.KtxTexture = nullptr;
    }

    TextureAsset& operator=(TextureAsset&& other) noexcept
    {
        if (this != &other)
        {
            Asset::operator=(std::move(other));
            if (KtxTexture)
            {
                ktxTexture2_Destroy(KtxTexture);
            }

            KtxTexture = other.KtxTexture;
            other.KtxTexture = nullptr;
        }
        return *this;
    }

    ktxTexture2* KtxTexture = nullptr;
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const TextureAsset& Value)
{
    using namespace LE::Archive;

    if (!Serialize(Ctx, Writer, static_cast<const Asset&>(Value)))
    {
        return false;
    }

    ktx_uint8_t* pDstBytes = nullptr;
    ktx_size_t size = 0;
    KTX_error_code result = ktxTexture2_WriteToMemory(Value.KtxTexture, &pDstBytes, &size);
    if (result != KTX_SUCCESS)
    {
        Ctx.Error.Raised = true;
        Ctx.Error.Desc = ktxErrorString(result);
        return false;
    }

    if (!Writer.WriteTrivialType(size))
    {
        return false;
    }

    if (!Writer.WriteBytes(pDstBytes, size))
    {
        return false;
    }

    return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, TextureAsset& Value)
{
    using namespace LE::Archive;

    if (!Deserialize(Ctx, Reader, static_cast<Asset&>(Value)))
    {
        return false;
    }

    ktx_size_t size = 0;
    if (!Reader.ReadTrivialType(size))
    {
        return false;
    }

    std::vector<ktx_uint8_t> pDstBytes;
    pDstBytes.resize(size);

    if (!Reader.ReadBytes(pDstBytes.data(), size))
    {
        return false;
    }

    KTX_error_code result = ktxTexture2_CreateFromMemory(pDstBytes.data(), size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &Value.KtxTexture);
    if (result != KTX_SUCCESS)
    {
        Ctx.Error.Raised = true;
        Ctx.Error.Desc = ktxErrorString(result);
        return false;
    }

    return true;
}

REGISTER_ASSET_TYPE(TextureAsset, "TextureAsset")
}
