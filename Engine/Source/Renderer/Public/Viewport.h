#pragma once

#include "RenderResource.h"

namespace LE
{
class GameViewport;
}

namespace LE::Renderer
{
class Viewport : public RenderResource
{
public:
	Viewport(GameViewport* Owner, RefCountingPtr<RHI::RHIViewport> ViewportRHI);

	void Draw();

	void EnqueueBeginRenderFrame();
	void EnqueueEndRenderFrame();

	void BeginRenderFrame(RenderCommandList& CmdList);
	void EndRenderFrame(RenderCommandList& CmdList);

	RefCountingPtr<RHI::RHIViewport> GetViewportRHI() const { return ViewportRHI; }

	void SetSizeX(uint32 InSizeX) { SizeX = InSizeX; }
	void SetSizeY(uint32 InSizeY) { SizeY = InSizeY; }
	void SetSizeXY(uint32 InSizeX, uint32 InSizeY) { SizeX = InSizeX; SizeY = InSizeY; }

	uint32 GetSizeX() const { return SizeX; }
	uint32 GetSizeY() const { return SizeY; }

protected:
	GameViewport* OwnerGameViewport;
	RefCountingPtr<RHI::RHIViewport> ViewportRHI;
	RefCountingPtr<RHI::RHITexture> RenderTargetTextureRHI;

	uint32 SizeX;
	uint32 SizeY;
};
}
