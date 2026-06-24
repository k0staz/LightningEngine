#include "FBXImporter.h"

#include "ofbx.h"
#include "FileManager/FileManager.h"
#include "Multithreading/JobScheduler.h"
#include "Service/ServiceRegistry.h"
#include "tracy/Tracy.hpp"

namespace LE
{
bool FBXImporter::LoadAndConvertFbxModel(const Path& FbxModelPath)
{
	std::unique_ptr<ConvertRequest> request = std::make_unique<ConvertRequest>();
	request->fbxPath = FbxModelPath;

	if(!LoadBinary(*request))
	{
		return false;
	}

	if(!ConvertToEngineType(*request))
	{
		return false;
	}

	if(!SaveStaticMesh(*request))
	{
		return false;
	}

	return true;
}

void FBXImporter::LoadAndConvertFbxModelAsync(const Path& FbxModelPath, const ImportCallback& Callback)
{
	std::unique_ptr<ConvertRequest> request = std::make_unique<ConvertRequest>();
	request->fbxPath = FbxModelPath;
	request->Callback = Callback;
	
	JobScheduler& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();
	RefCountingPtr<AsyncTaskNodeBase> loadingTaskNode = MultithreadingUtils::MakeTask("BinaryLoadFBXStaticMeshTask", &jobScheduler, &FBXImporter::LoadBinaryTask, std::move(request));
	loadingTaskNode->Finalize();
}

bool FBXImporter::LoadBinary(ConvertRequest& Request)
{
	Request.LoadedSceneBinary = std::make_unique<std::vector<std::byte>>();
	if(!LoadFile(Request.fbxPath, *Request.LoadedSceneBinary))
	{
		LE_ASSERT_DESC(false, "Failed to open model at path: {}, when using FBX importer", Request.fbxPath.string())
		return false;
	}
	
	return true;
}

bool FBXImporter::ConvertToEngineType(ConvertRequest& Request)
{
	if(Request.LoadedSceneBinary->empty())
	{
		return false;
	}
	
	static ofbx::LoadFlags flags =
		ofbx::LoadFlags::IGNORE_BLEND_SHAPES
		| ofbx::LoadFlags::IGNORE_CAMERAS
		| ofbx::LoadFlags::IGNORE_LIGHTS
		| ofbx::LoadFlags::IGNORE_TEXTURES
		| ofbx::LoadFlags::IGNORE_SKIN
		| ofbx::LoadFlags::IGNORE_BONES
		| ofbx::LoadFlags::IGNORE_PIVOTS
		| ofbx::LoadFlags::IGNORE_ANIMATIONS
		| ofbx::LoadFlags::IGNORE_MATERIALS
		| ofbx::LoadFlags::IGNORE_POSES
		| ofbx::LoadFlags::IGNORE_VIDEOS
		| ofbx::LoadFlags::IGNORE_LIMBS;
	
	const ofbx::u8* rawData = reinterpret_cast<ofbx::u8*>(Request.LoadedSceneBinary->data());
	ofbx::IScene* scene = ofbx::load(rawData, static_cast<ofbx::usize>(Request.LoadedSceneBinary->size()), static_cast<ofbx::u16>(flags));
	if(!scene)
	{
		LE_ASSERT_DESC(false, "Failed to load model at path {}", Request.fbxPath.string())
		return false;
	}

	const int meshCount = scene->getMeshCount();
	
	std::vector<StaticMeshObject>& convertedAssets = Request.LoadedMeshes;
	convertedAssets.clear();
	convertedAssets.reserve(meshCount);
	for(int i = 0; i < meshCount; ++i)
	{
		std::unique_ptr<StaticMeshAsset> meshAsset = std::make_unique<StaticMeshAsset>(AssetIdNull, Uid::GenerateUid(), AssetTypeIdGetter<StaticMeshAsset>::Value);
		meshAsset->PrimitiveType = RHI::PrimitiveType::TriangleList;
		
		const ofbx::Mesh& mesh = *scene->getMesh(i);
		const ofbx::GeometryData& geometryData = mesh.getGeometryData();
		const ofbx::Vec3Attributes positions = geometryData.getPositions();
		const ofbx::Vec3Attributes normals = geometryData.getNormals();
		const ofbx::Vec2Attributes uvs = geometryData.getUVs();

		// Create vertices
		meshAsset->Vertices.resize(positions.count);
		for(int vertexIdx = 0; vertexIdx < positions.count; ++vertexIdx)
		{
			Renderer::StaticMeshVertex& vertex = meshAsset->Vertices[vertexIdx];
			ofbx::Vec3 ofbxPosition = positions.get(vertexIdx);
			vertex.Position = {ofbxPosition.x, ofbxPosition.y, ofbxPosition.z};
			
			if(normals.values)
			{
				ofbx::Vec3 n = normals.get(vertexIdx);
				vertex.Normal = Vector3F(n.x, n.y, n.z);
			}
			
			if(uvs.values)
			{
				ofbx::Vec2 uv = uvs.get(vertexIdx);
				vertex.TextureCord = {uv.x, uv.y};
			}
		}

		// Partition - material
		for(int partitionIdx = 0; partitionIdx < geometryData.getPartitionCount(); ++partitionIdx)
		{
			const ofbx::GeometryPartition& partition = geometryData.getPartition(partitionIdx);
			for(int polygonIdx = 0; polygonIdx < partition.polygon_count; ++polygonIdx)
			{
				const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygonIdx];
				// Triangulate polygon, as it is not necessary triangle
				int indices[128];
				const int vertexCount = ofbx::triangulate(geometryData, polygon, indices);
				
				for(int vertexIdx = 0; vertexIdx < vertexCount; ++vertexIdx)
				{
					const uint32 vertexIndex = static_cast<uint32>(indices[vertexIdx]);
					meshAsset->Indices.emplace_back(vertexIndex);
				}
			}
		}

		convertedAssets.emplace_back(mesh.name, std::move(meshAsset));
	}
	scene->destroy();

	return true;
}

bool FBXImporter::SaveStaticMesh(ConvertRequest& Request)
{
	const Path savePath = GetContentRoot() / "StaticMesh";
	const std::string extensionStr = ".leasset";
	AssetRegistry& reg = GetServiceRegistry().GetService<AssetRegistry>();
	for(StaticMeshObject& convertedStaticMesh : Request.LoadedMeshes)
	{

		Path meshSavePath = savePath / (convertedStaticMesh.Name + extensionStr);
		Uid existingAssetUid = reg.GetUidFromPath(meshSavePath);
		uint32 copyCounter = 1;
		while(existingAssetUid.IsValid())
		{
			meshSavePath = savePath / (convertedStaticMesh.Name + std::to_string(copyCounter++) + extensionStr);
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
		if(!Archive::Serialize(context, archiveWriter, meshAsset))
		{
			LE_ASSERT_DESC(false, "Failed to serialize converted mesh {} at the path {}, due to {}", convertedStaticMesh.Name, meshSavePath.string(), context.Error.Desc)
		}
		else if(!SaveFile(meshSavePath, writeBuffer))
		{
			LE_ASSERT_DESC(false, "Failed to serialize converted mesh {} at the path {}", convertedStaticMesh.Name, meshSavePath.string())
		}
	}
	
	reg.SaveManifest();

	return true;
}

void FBXImporter::LoadBinaryTask(std::unique_ptr<ConvertRequest> Request)
{
	ZoneScopedNC("FBXImported::LoadBinary", tracy::Color::Purple);
	if(!LoadBinary(*Request))
	{
		Request->Finish(false);
	}

	JobScheduler& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();
	RefCountingPtr<AsyncTaskNodeBase> loadingTaskNode = MultithreadingUtils::MakeTask(
		"ConvertFBXStaticMeshTask",
		&jobScheduler,
		&FBXImporter::ConvertToEngineTypeTask,
		std::move(Request));
	
	loadingTaskNode->Finalize();
}

void FBXImporter::ConvertToEngineTypeTask(std::unique_ptr<ConvertRequest> Request)
{
	ZoneScopedNC("FBXImported::ConvertToEngineType", tracy::Color::Purple);
	if(!ConvertToEngineType(*Request))
	{
		Request->Finish(false);
	}

	JobScheduler& jobScheduler = GetServiceRegistry().GetService<JobScheduler>();
	RefCountingPtr<AsyncTaskNodeBase> loadingTaskNode = MultithreadingUtils::MakeTask(
		"SaveFBXStaticMeshTask",
		&jobScheduler,
		&FBXImporter::SaveStaticMeshTask,
		std::move(Request));
	
	loadingTaskNode->Finalize();
}

void FBXImporter::SaveStaticMeshTask(std::unique_ptr<ConvertRequest> Request)
{
	ZoneScopedNC("FBXImported::SaveStaticMesh", tracy::Color::Purple);
	if(!SaveStaticMesh(*Request))
	{
		Request->Finish(false);
	}

	Request->Finish(true);
}
}
