#pragma once
#include "RenderCommandList.h"
#include "RenderProxyStorage.h"
#include "AssetManager/AssetManager.h"
#include "Assets/StaticMeshAsset.h"
#include "ECS/EcsEntity.h"
#include "Math/Matrix4x4.h"
#include "Multithreading/SharedResource.h"
#include "Templates/RefCounters.h"


namespace LE::Renderer
{
class RenderScene : public RefCountableBase
{
public:
	RenderScene() = default;
	RenderScene(RenderScene&&) = delete;
	RenderScene(const RenderScene&) = delete;
	RenderScene& operator=(const RenderScene&) = delete;
	RenderScene& operator=(RenderScene&&) = delete;
	~RenderScene() override;

	bool HasRenderProxy(EcsEntity EntityId) const;
	
	void CreateStaticMeshRenderProxy(EcsEntity EntityId, AssetHandle<StaticMeshAsset> MeshAsset, const Matrix4x4F& Transform);
	void UpdateStaticMeshRenderProxy(EcsEntity EntityId, const Matrix4x4F& Transform);
	
	void DeleteRenderProxy(EcsEntity EntityId);
	void SetRenderProxyEnabled(EcsEntity EntityId, bool Enabled);
	
	void GetEnabledRenderProxies(std::vector<RenderProxyState*>& EnabledProxies);
private:
	RenderProxyStorage<EcsEntity> RenderProxies;
	
};
}
