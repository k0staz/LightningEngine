#pragma once

#include <shared_mutex>

#include "Assets/MaterialInstanceAsset.h"
#include "Assets/StaticMeshAsset.h"
#include "Assets/TextureAsset.h"
#include "Misc/Paths.h"
#include "Misc/Uid.h"
#include "Service/ServiceBase.h"

namespace LE
{
class GLTFImporter : public ServiceBase
{
public:

    using ImportCallback = std::function<void(bool, std::vector<Uid>)>;

    void Initialize() override {}
    void Shutdown() override {}

    static bool LoadAndConvertModel(const Path& ModelPath);
    static void LoadAndConvertModelAsync(const Path& ModelPath, const ImportCallback& Callback = nullptr);

private:
    struct StaticMeshObject
    {
        std::string Name;
        std::unique_ptr<StaticMeshAsset> MeshAsset;
    };

    struct MaterialObject
    {
        std::string Name;
        int32 ColorTextureIndex = -1;
        int32 NormalTextureIndex = -1;
        int32 HeightTextureIndex = -1;
    };

    struct TextureObject
    {
        std::string Name;
        std::unique_ptr<TextureAsset> TexAsset;
    };

    struct ConvertRequest
    {
        void Finish(const bool IsSuccess) const
        {
            if(Callback)
            {
                std::vector<Uid> convertedUids;
                if(IsSuccess)
                {
                    convertedUids.reserve(LoadedMeshes.size());
                    for(const StaticMeshObject& asset : LoadedMeshes)
                    {
                        convertedUids.emplace_back(asset.MeshAsset->GetStableId());
                    }
                }

                Callback(IsSuccess, convertedUids);
            }
        }

        Path ModelPath;
        std::unique_ptr<std::vector<std::byte>> LoadedSceneBinary = nullptr;
        std::vector<StaticMeshObject> LoadedMeshes;
        std::vector<MaterialObject> LoadedMaterials;
        std::vector<TextureObject> LoadedTextures;
        std::unordered_map<std::string, size_t> LoadedTexturesMap;
        ImportCallback Callback = nullptr;
    };

    static bool LoadBinary(ConvertRequest& Request);
    static bool ConvertToEngineType(ConvertRequest& Request);
    static bool SaveStaticMesh(ConvertRequest& Request);

    static void LoadBinaryTask(std::unique_ptr<ConvertRequest> Request);
    static void ConvertToEngineTypeTask(std::unique_ptr<ConvertRequest> Request);
    static void SaveStaticMeshTask(std::unique_ptr<ConvertRequest> Request);
};

REGISTER_SERVICE_TYPE(GLTFImporter, "GLTFImporter")
}
