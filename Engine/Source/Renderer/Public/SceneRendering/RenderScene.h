#pragma once
#include "ECS/EcsEntity.h"
#include "StaticMesh/StaticMeshRendering.h"
#include "Templates/RefCounters.h"


namespace LE::Renderer
{
class RenderObjectProxy;

class RenderScene : public RefCountableBase
{
public:
	RenderScene() = default;
	RenderScene(RenderScene&&) = delete;
	RenderScene(const RenderScene&) = delete;
	RenderScene& operator=(const RenderScene&) = delete;
	RenderScene& operator=(RenderScene&&) = delete;
	~RenderScene() override;

	const Map<EcsEntity, RenderObjectProxy*>& GetProxyMap() const { return RenderObjectProxies; }

	void CreateStaticMeshRenderProxy(EcsEntity Entity, const Matrix4x4F& Transform, const StaticMeshRenderData* RenderData, const Material* MeshMaterial);
	void DeleteRenderObjectProxy(EcsEntity Entity);
	void UpdateStaticMeshProxyTransform(EcsEntity Entity, const Matrix4x4F& Transform);

private:
	Map<EcsEntity, RenderObjectProxy*> RenderObjectProxies;
};
}
