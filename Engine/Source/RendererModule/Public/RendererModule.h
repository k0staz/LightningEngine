#pragma once
#include "Module.h"
#include "RHIDevice.h"
#include "SceneRendering/RenderScene.h"
#include "LEWindow.h"
#include "SceneRendering/SceneView.h"

namespace LE
{
class RendererModule : public ModuleBase
{
public:
	void RegisterServices() override;
	void RegisterReflection() override;
	void ShutdownServices() override;
	
	void InitializeWithVulkanDevice();
	
	void InitializeGlobalFrameData(); 

	Renderer::RenderScene& GetRenderScene();
	
	void CreateRHIWindow(RefCountingPtr<const LEWindow> Window);
	void DeleteRHIWindow(RefCountingPtr<const LEWindow> Window);
	RefCountingPtr<RHI::RHIWindow> GetRhiWindow(RefCountingPtr<const LEWindow> Window);

	void AddFrame(RefCountingPtr<const LEWindow> Window, const Renderer::SceneViewInfo& ViewInfo);
	void ScheduleDrawFrame();

protected:
	void DrawFrameInternal(float);
	
	void EnqueueFrameSceneView(Renderer::SceneView View);
	Renderer::SceneView GetFrameSceneView();

public:
	void Shutdown() override;

private:
	std::unique_ptr<RHI::RHIDevice> Device;
	Renderer::RenderContributorId GlobalFrameDataContributorId = NullId{};
	
	Map<RefCountingPtr<const LEWindow>, RefCountingPtr<RHI::RHIWindow>> RhiWindowMap;
	Renderer::RenderScene Scene;
	
	std::vector<Renderer::SceneView> FrameSceneViews;
	std::mutex SceneViewMutex;
};

REGISTER_MODULE(RendererModule, "RendererModule")
}
