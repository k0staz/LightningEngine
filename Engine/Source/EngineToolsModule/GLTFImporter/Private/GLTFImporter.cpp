#include "GLTFImporter.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "../../ImageDecoder/Public/ImageDecoder.h"

#include "FileManager/FileManager.h"
#include "Multithreading/JobScheduler.h"
#include "Service/ServiceRegistry.h"
#include "tracy/Tracy.hpp"

namespace LE
{
bool GLTFImporter::LoadAndConvertModel(const Path& ModelPath)
{
    std::unique_ptr<ConvertRequest> request = std::make_unique<ConvertRequest>();
    request->ModelPath = ModelPath;

    if (!LoadBinary(*request))
    {
        return false;
    }

    if (!ConvertToEngineType(*request))
    {
        return false;
    }

    if(!SaveStaticMesh(*request))
    {
        return false;
    }

    return true;
}

void GLTFImporter::LoadAndConvertModelAsync(const Path& ModelPath, const ImportCallback& Callback)
{
    std::unique_ptr<ConvertRequest> request = std::make_unique<ConvertRequest>();
    request->ModelPath = ModelPath;
    request->Callback = Callback;

    auto& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();
    RefCountingPtr<AsyncTaskNodeBase> loadingTaskNode = MultithreadingUtils::MakeTask(
        "BinaryLoadFBXStaticMeshTask", &jobScheduler, &GLTFImporter::LoadBinaryTask, std::move(request));
    loadingTaskNode->Finalize();
}

bool GLTFImporter::LoadBinary(ConvertRequest& Request)
{
    Request.LoadedSceneBinary = std::make_unique<std::vector<std::byte>>();
    if (!LoadFile(Request.ModelPath, *Request.LoadedSceneBinary))
    {
        LE_ASSERT_DESC(false, "Failed to open model at path: {}, when using FBX importer", Request.ModelPath.string())
        return false;
    }

    return true;
}

#define CHECK_CGLTF_RESULT(Result, Data, ErrorText, ...) \
    if (Result != cgltf_result_success) \
    { \
        cgltf_free(Data); \
        LE_ERROR(ErrorText, __VA_ARGS__); \
        return false; \
    }

bool GLTFImporter::ConvertToEngineType(ConvertRequest& Request)
{
    if (Request.LoadedSceneBinary->empty())
    {
        return false;
    }

    cgltf_options options = {};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&options, Request.LoadedSceneBinary->data(), Request.LoadedSceneBinary->size(), &data);
    CHECK_CGLTF_RESULT(result, data, "Failed to parse gltf file: {}", Request.ModelPath.string());

    result = cgltf_load_buffers(&options, data, nullptr);
    CHECK_CGLTF_RESULT(result, data, "Failed to load buffers for gltf file: {}", Request.ModelPath.string());

    const cgltf_size meshCount = data->meshes_count;
    std::vector<StaticMeshObject>& convertedMeshes = Request.LoadedMeshes;
    convertedMeshes.reserve(meshCount);
    for (cgltf_size i = 0; i < meshCount; ++i)
    {
        const cgltf_mesh* mesh = &data->meshes[i];
        if (!mesh)
        {
            LE_ERROR("Invalid mesh at index {}", i);
            continue;
        }

        StaticMeshObject& convertedStaticMesh = convertedMeshes.emplace_back();
        convertedStaticMesh.Name = mesh->name;

        const cgltf_size primCount = mesh->primitives_count;
        LE_ASSERT_DESC(primCount == 1, "Only single primitive meshes are supported right now");
        const cgltf_primitive& primitive = mesh->primitives[0];
        if (primitive.type != cgltf_primitive_type_triangles)
        {
            LE_ERROR("Only triangle meshes are supported right now");
            continue;
        }

        std::unique_ptr<StaticMeshAsset> meshAsset = std::make_unique<StaticMeshAsset>(
            AssetIdNull, Uid::GenerateUid(), AssetTypeIdGetter<StaticMeshAsset>::Value);
        meshAsset->PrimitiveType = RHI::PrimitiveType::TriangleList;

        meshAsset->Indices.resize(primitive.indices->count);
        for (cgltf_size index = 0; index < primitive.indices->count; ++index)
        {
            meshAsset->Indices[index] = static_cast<uint32>(cgltf_accessor_read_index(primitive.indices, index));
        }

        const cgltf_size attributeCount = primitive.attributes_count;

        for (int attributeIdx = 0; attributeIdx < attributeCount; ++attributeIdx)
        {
            const cgltf_attribute& attribute = primitive.attributes[attributeIdx];
            if (attribute.type == cgltf_attribute_type_position)
            {
                const cgltf_size posCount = attribute.data->count;
                meshAsset->Positions.resize(posCount);
                for (cgltf_size posIdx = 0; posIdx < posCount; ++posIdx)
                {
                    Vector3F& position = meshAsset->Positions[posIdx];
                    cgltf_accessor_read_float(attribute.data, posIdx, reinterpret_cast<cgltf_float*>(&position), 3);
                }
            }
            else if (attribute.type == cgltf_attribute_type_normal)
            {
                const cgltf_size normalCount = attribute.data->count;
                meshAsset->Normals.resize(normalCount);
                for (cgltf_size normalIdx = 0; normalIdx < normalCount; ++normalIdx)
                {
                    Vector3F& normal = meshAsset->Normals[normalIdx];
                    cgltf_accessor_read_float(attribute.data, normalIdx, reinterpret_cast<cgltf_float*>(&normal), 3);
                }
            }
            else if (attribute.type == cgltf_attribute_type_texcoord)
            {
                const cgltf_size texCoordCount = attribute.data->count;
                meshAsset->UVs.resize(texCoordCount);
                for (cgltf_size texCoordIdx = 0; texCoordIdx < texCoordCount; ++texCoordIdx)
                {
                    Vector2F& texCoord = meshAsset->UVs[texCoordIdx];
                    cgltf_accessor_read_float(attribute.data, texCoordIdx, reinterpret_cast<cgltf_float*>(&texCoord), 2);
                }
            }
        }

        convertedStaticMesh.MeshAsset = std::move(meshAsset);
    }

    const cgltf_size textureCount = data->textures_count;
    std::vector<TextureObject>& loadedTextures = Request.LoadedTextures;
    loadedTextures.reserve(textureCount);
    std::unordered_map<std::string, size_t>& loadedTexturesMap = Request.LoadedTexturesMap;
    for (cgltf_size textureIdx = 0; textureIdx < textureCount; ++textureIdx)
    {
        const cgltf_texture& texture = data->textures[textureIdx];
        const cgltf_image* image = texture.image;
        const std::string textureName = image->name;
        const cgltf_buffer_view* imageBufferView = image->buffer_view;
        const uint8* pngData = static_cast<const uint8*>(imageBufferView->buffer->data) + imageBufferView->offset;

        std::span<const uint8> bufferViewData(pngData, static_cast<size_t>(image->buffer_view->size));
        ImageUtils::DecodedPNG decodedPng;
        if (!ImageUtils::DecodeBinaryPNG(bufferViewData, decodedPng))
        {
            continue;
        }

        ktxTextureCreateInfo textureCreateInfo = {};
        if (decodedPng.Channels == 1)
        {
            textureCreateInfo.vkFormat = 9; // VK_FORMAT_R8_UNORM
        }
        else if (decodedPng.Channels == 3)
        {
            textureCreateInfo.vkFormat = 23; // VK_FORMAT_R8G8B8_UNORM
        }
        else
        {
            textureCreateInfo.vkFormat = 37; // VK_FORMAT_R8G8B8A8_UNORM
        }
        textureCreateInfo.baseWidth = decodedPng.Width;
        textureCreateInfo.baseHeight = decodedPng.Height;
        textureCreateInfo.baseDepth = 1;
        textureCreateInfo.numDimensions = 2;
        textureCreateInfo.numLevels = 1;
        textureCreateInfo.numLayers = 1;
        textureCreateInfo.numFaces = 1;
        textureCreateInfo.isArray = KTX_FALSE;
        textureCreateInfo.generateMipmaps = KTX_TRUE;

        ktxTexture2* ktx_tex = nullptr;
        if (ktxTexture2_Create(&textureCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktx_tex) != KTX_SUCCESS)
        {
            LE_ERROR("Failed to create KTX texture for: {}", textureName);
            continue;
        }

        if (ktxTexture_SetImageFromMemory(ktxTexture(ktx_tex), 0, 0, 0, decodedPng.Data.data(), decodedPng.Data.size()) != KTX_SUCCESS)
        {
            ktxTexture2_Destroy(ktx_tex);
            LE_ERROR("Failed to set image for KTX texture for: {}", textureName);
            continue;
        }

        std::unique_ptr<TextureAsset> textureAsset = std::make_unique<TextureAsset>(
            AssetIdNull, Uid::GenerateUid(), AssetTypeIdGetter<TextureAsset>::Value);
        textureAsset->KtxTexture = ktx_tex;
        loadedTextures.emplace_back(textureName, std::move(textureAsset));
        loadedTexturesMap[textureName] = loadedTextures.size() - 1;
    }

    const cgltf_size materialCount = data->materials_count;
    std::vector<MaterialObject>& loadedMaterials = Request.LoadedMaterials;
    loadedMaterials.reserve(materialCount);
    for (cgltf_size i = 0; i < materialCount; ++i)
    {
        const cgltf_material& material = data->materials[i];
        const std::string materialName = material.name;

        const std::string baseColorTextureName = material.pbr_metallic_roughness.base_color_texture.texture->image->name;
        const std::string displacementTextureName = material.occlusion_texture.texture->image->name;
        const std::string normalTextureName = material.normal_texture.texture->image->name;

        MaterialObject materialObject = {};
        materialObject.Name = materialName;
        if (loadedTexturesMap.contains(baseColorTextureName))
        {
            materialObject.ColorTextureIndex = static_cast<int32>(loadedTexturesMap[baseColorTextureName]);
        }
        if (loadedTexturesMap.contains(displacementTextureName))
        {
            materialObject.HeightTextureIndex = static_cast<int32>(loadedTexturesMap[displacementTextureName]);
        }
        if (loadedTexturesMap.contains(normalTextureName))
        {
            materialObject.NormalTextureIndex = static_cast<int32>(loadedTexturesMap[normalTextureName]);
        }

        loadedMaterials.emplace_back(materialObject);
    }

}

bool GLTFImporter::SaveStaticMesh(ConvertRequest& Request)
{
    const Path savePath = GetContentRoot();
    const std::string extensionStr = ".leasset";
    auto& reg = GetServiceRegistry().GetService<AssetRegistry>();

    const Path meshesSavePath = savePath / "StaticMesh";
    for (StaticMeshObject& convertedStaticMesh : Request.LoadedMeshes)
    {
        Path meshSavePath = meshesSavePath / (convertedStaticMesh.Name + extensionStr);
        Uid existingAssetUid = reg.GetUidFromPath(meshSavePath);
        uint32 copyCounter = 1;
        while (existingAssetUid.IsValid())
        {
            meshSavePath = meshesSavePath / (convertedStaticMesh.Name + std::to_string(copyCounter++) + extensionStr);
            existingAssetUid = reg.GetUidFromPath(meshSavePath);
        }

        StaticMeshAsset& meshAsset = *convertedStaticMesh.MeshAsset;
        AssetInfo newInfo;
        newInfo.PathToAsset = meshSavePath;
        newInfo.AssetUid = meshAsset.GetStableId();
        newInfo.TypeId = AssetTypeIdGetter<StaticMeshAsset>::Value;
        reg.SetAssetInfo(newInfo);

        Archive::Context context(&newInfo);

        std::vector<std::byte> writeBuffer;
        Archive::ArchiveWriter archiveWriter(writeBuffer);
        if (!Archive::Serialize(context, archiveWriter, meshAsset))
        {
            LE_ASSERT_DESC(false, "Failed to serialize converted mesh {} at the path {}, due to {}", convertedStaticMesh.Name,
                           meshSavePath.string(), context.Error.Desc)
            continue;
        }

        if (!SaveFile(meshSavePath, writeBuffer))
        {
            LE_ASSERT_DESC(false, "Failed to serialize converted mesh {} at the path {}", convertedStaticMesh.Name, meshSavePath.string())
        }
    }

    const Path texturesSavePath = savePath / "Textures";
    for (TextureObject& convertedTexture : Request.LoadedTextures)
    {
        Path textureSavePath = texturesSavePath / (convertedTexture.Name + extensionStr);
        Uid existingAssetUid = reg.GetUidFromPath(textureSavePath);
        uint32 copyCounter = 1;
        while (existingAssetUid.IsValid())
        {
            textureSavePath = texturesSavePath / (convertedTexture.Name + std::to_string(copyCounter++) + extensionStr);
            existingAssetUid = reg.GetUidFromPath(textureSavePath);
        }

        TextureAsset& textureAsset = *convertedTexture.TexAsset;
        AssetInfo newInfo;
        newInfo.PathToAsset = textureSavePath;
        newInfo.AssetUid = textureAsset.GetStableId();
        newInfo.TypeId = AssetTypeIdGetter<TextureAsset>::Value;
        reg.SetAssetInfo(newInfo);

        Archive::Context context(&newInfo);

        std::vector<std::byte> writeBuffer;
        Archive::ArchiveWriter archiveWriter(writeBuffer);
        if (!Archive::Serialize(context, archiveWriter, textureAsset))
        {
            LE_ASSERT_DESC(false, "Failed to serialize converted texture {} at the path {}, due to {}", convertedTexture.Name,
                           textureSavePath.string(), context.Error.Desc)
            continue;
        }

        if (!SaveFile(textureSavePath, writeBuffer))
        {
            LE_ASSERT_DESC(false, "Failed to serialize converted texture {} at the path {}", convertedTexture.Name, textureSavePath.string())
        }
    }

    const Path materialsSavePath = savePath / "Materials";
    for (MaterialObject& convertedMaterial : Request.LoadedMaterials)
    {
        Path materialSavePath = materialsSavePath / (convertedMaterial.Name + extensionStr);
        Uid existingAssetUid = reg.GetUidFromPath(materialSavePath);
        uint32 copyCounter = 1;
        while (existingAssetUid.IsValid())
        {
            materialSavePath = materialsSavePath / (convertedMaterial.Name + std::to_string(copyCounter++) + extensionStr);
            existingAssetUid = reg.GetUidFromPath(materialSavePath);
        }

        AssetInfo newInfo;
        newInfo.PathToAsset = materialSavePath;
        newInfo.AssetUid = Uid::GenerateUid();
        newInfo.TypeId = AssetTypeIdGetter<MaterialInstanceAsset>::Value;

        Asset materialAsset(AssetIdNull, newInfo.AssetUid, newInfo.TypeId);

        Archive::Context context(&newInfo);
        std::vector<std::byte> writeBuffer;
        Archive::ArchiveWriter archiveWriter(writeBuffer);
        if (!Archive::Serialize(context, archiveWriter, materialAsset))
        {
            LE_ERROR("Failed to serialize material asset: {}", convertedMaterial.Name);
            continue;
        }

        auto serializeTexture = [&context, &archiveWriter, &Request, &convertedMaterial, &newInfo](const int32 Index)
        {
            if (Index >= 0)
            {
                const auto& textureAsset = Request.LoadedTextures[Index];
                if (!Archive::Serialize(context, archiveWriter, textureAsset.TexAsset->GetStableId()))
                {
                    LE_ERROR("Failed to serialize texture asset: {}, when serializing material asset: {}", textureAsset.Name, convertedMaterial.Name);
                    return false;
                }
                newInfo.Dependencies.emplace(textureAsset.TexAsset->GetStableId());
            }
            else
            {
                if (!Archive::Serialize(context, archiveWriter, EmptyUid))
                {
                    LE_ERROR("Failed to serialize empty texture asset when serializing material asset: {}", convertedMaterial.Name);
                    return false;
                }
            }

            return true;
        };

        if (!serializeTexture(convertedMaterial.ColorTextureIndex))
        {
            continue;
        }

        if (!serializeTexture(convertedMaterial.NormalTextureIndex))
        {
            continue;
        }

        if (!serializeTexture(convertedMaterial.HeightTextureIndex))
        {
            continue;
        }

        reg.SetAssetInfo(newInfo);

        if (!SaveFile(materialSavePath, writeBuffer))
        {
            LE_ASSERT_DESC(false, "Failed to serialize converted texture {} at the path {}", convertedMaterial.Name, materialSavePath.string())
        }
    }

    reg.SaveManifest();

    return true;
}

void GLTFImporter::LoadBinaryTask(std::unique_ptr<ConvertRequest> Request)
{
    ZoneScopedNC("GLTFImporter::LoadBinary", tracy::Color::Purple);
    if (!LoadBinary(*Request))
    {
        Request->Finish(false);
    }

    auto& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();
    RefCountingPtr<AsyncTaskNodeBase> loadingTaskNode = MultithreadingUtils::MakeTask(
        "ConvertFBXStaticMeshTask",
        &jobScheduler,
        &GLTFImporter::ConvertToEngineTypeTask,
        std::move(Request));

    loadingTaskNode->Finalize();
}

void GLTFImporter::ConvertToEngineTypeTask(std::unique_ptr<ConvertRequest> Request)
{
    ZoneScopedNC("GLTFImporter::ConvertToEngineType", tracy::Color::Purple);
    if (!ConvertToEngineType(*Request))
    {
        Request->Finish(false);
    }

    auto& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();
    RefCountingPtr<AsyncTaskNodeBase> loadingTaskNode = MultithreadingUtils::MakeTask(
        "SaveFBXStaticMeshTask",
        &jobScheduler,
        &GLTFImporter::SaveStaticMeshTask,
        std::move(Request));

    loadingTaskNode->Finalize();
}

void GLTFImporter::SaveStaticMeshTask(std::unique_ptr<ConvertRequest> Request)
{
    ZoneScopedNC("GLTFImporter::SaveStaticMesh", tracy::Color::Purple);
    if (!SaveStaticMesh(*Request))
    {
        Request->Finish(false);
    }

    Request->Finish(true);
}
}