#pragma once

#include <shared_mutex>

#include "StaticMeshAsset.h"
#include "Misc/Paths.h"
#include "Misc/Uid.h"
#include "Service/ServiceBase.h"

namespace LE
{
class FBXImporter : public ServiceBase
{
public:

	using ImportCallback = std::function<void(bool, std::vector<Uid>)>;
	
	void Initialize() override {}
	void Shutdown() override {}
	
	static bool LoadAndConvertFbxModel(const Path& FbxModelPath);
	static void LoadAndConvertFbxModelAsync(const Path& FbxModelPath, const ImportCallback& Callback = nullptr);
	
private:
	struct StaticMeshObject
	{
		std::string Name;
		std::unique_ptr<StaticMeshAsset> MeshAsset;
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
		
		Path fbxPath;
		std::unique_ptr<std::vector<std::byte>> LoadedSceneBinary = nullptr;
		std::vector<StaticMeshObject> LoadedMeshes;
		ImportCallback Callback = nullptr;
	};
	
	static bool LoadBinary(ConvertRequest& Request);
	static bool ConvertToEngineType(ConvertRequest& Request);
	static bool SaveStaticMesh(ConvertRequest& Request);

	static void LoadBinaryTask(std::unique_ptr<ConvertRequest> Request);
	static void ConvertToEngineTypeTask(std::unique_ptr<ConvertRequest> Request);
	static void SaveStaticMeshTask(std::unique_ptr<ConvertRequest> Request);
};

REGISTER_SERVICE_TYPE(FBXImporter, "FBXImporter")
}
