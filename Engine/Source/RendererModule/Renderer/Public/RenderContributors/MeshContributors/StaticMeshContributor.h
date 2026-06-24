#pragma once
#include "Math/Matrix4x4.h"
#include "RenderContributors/RenderContributor.h"
#include "RenderContributors/RenderContributorInstanceStorage.h"
#include "RenderResourceManager/RenderResourceManager.h"
#include "RenderResourceManager/RenderResources.h"
#include "Templates/Alignment.h"

namespace LE::Renderer
{
/**
 * @brief StaticMeshContributor registers as a render contributor to handle static mesh rendering.
 *
 * The class manages static mesh-related resources and their rendering state. It provides
 * mechanisms for storing and updating dynamic data, handling proxies for entities, and
 * generating per-frame render data. StaticMeshContributor interacts with the rendering
 * pipeline to supply necessary data for drawing static meshes to the scene.
 */
class StaticMeshContributor : public RenderContributor
{
public:
	StaticMeshContributor() = default;
	StaticMeshContributor(RenderContributorId InInstanceId, RenderContributorTypeId InTypeId)
		: RenderContributor(InInstanceId, InTypeId)
	{}

	~StaticMeshContributor() override;
	
	StaticMeshContributor(StaticMeshContributor&&) = default;
	StaticMeshContributor& operator=(StaticMeshContributor&&) = default;

	RenderContributorMetaType GetMetaType() const override { return RenderContributorMetaType::MeshData; }

	struct StaticMeshDynamicData
	{
		Matrix4x4F LocalToWorld;
	};

	struct StaticMeshResources
	{
		uint64 IndicesFetchPtr;
		uint64 PositionsFetchPtr;
		uint64 NormalsFetchPtr;
		uint64 TexCoordsFetchPtr;
	};

	struct StaticMeshFrameData
	{
		uint64 ResourceDataGpuAddress;
		uint64 DynamicDataGpuAddress;
	};

	StaticMeshDynamicData& GetDynamicData(EcsEntity Entity);
	void SetRenderResourceId(RenderResourceHandle<const StaticMeshRenderResource> Resource);

	bool HasProxy(EcsEntity Entity) override;
	void AddProxy(EcsEntity Entity) override;
	void RemoveProxy(EcsEntity Entity) override;

	void WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer) override;
	void AddProxyToThisFrameContribution(EcsEntity Entity) override;

	uint32 GetIndexCount() const override;
	bool IsReady() const override;

private:
	bool IsStaticMeshResourceReady() const;

private:
	RenderResourceHandle<const StaticMeshRenderResource> RenderResource = {};
	RenderContributorInstanceStorage<StaticMeshDynamicData, EcsEntity> ProxyDynamicDataStorage;
	std::vector<StaticMeshDynamicData*> ThisFrameDynamicData;
};

REGISTER_RENDER_CONTRIBUTOR_TYPE(LE::Renderer::StaticMeshContributor, "StaticMeshFetch", "IMeshFetch", "Mesh/StaticMesh/StaticMesh.slang")
}
