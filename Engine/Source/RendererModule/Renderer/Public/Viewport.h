#pragma once

#include "RenderResource.h"

namespace LE::Renderer
{
class Viewport : public RenderResource
{
public:
	Viewport(RefCountingPtr<RHI::RHIViewport> ViewportRHI);
	
	void EnqueueBeginRenderFrame();
	void EnqueueEndRenderFrame();

	void BeginRenderFrame(RenderCommandList& CmdList);
	void EndRenderFrame(RenderCommandList& CmdList);

	RefCountingPtr<RHI::RHIViewport> GetViewportRHI() const { return ViewportRHI; }

	void SetSizeX(uint32 InSizeX) { ViewportRHI->SetSizeX(InSizeX); }
	void SetSizeY(uint32 InSizeY) { ViewportRHI->SetSizeY(InSizeY); }
	void SetSizeXY(uint32 InSizeX, uint32 InSizeY) { ViewportRHI->SetSizeXY(InSizeX, InSizeY); }

	uint32 GetSizeX() const { return ViewportRHI->GetSizeX(); }
	uint32 GetSizeY() const { return ViewportRHI->GetSizeY(); }

protected:
	RefCountingPtr<RHI::RHIViewport> ViewportRHI;
	RefCountingPtr<RHI::RHITexture> RenderTargetTextureRHI;
};
}
