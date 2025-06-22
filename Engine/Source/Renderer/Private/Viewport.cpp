#include "Viewport.h"

#include "DynamicRHI.h"
#include "GameViewport.h"
#include "RenderCommandList.h"

namespace LE::Renderer
{
Viewport::Viewport(GameViewport* Owner, RefCountingPtr<RHI::RHIViewport> ViewportRHI)
	: OwnerGameViewport(Owner)
	  , ViewportRHI(ViewportRHI)
	  , SizeX(0)
	  , SizeY(0)
{
}

void Viewport::Draw()
{
	EnqueueBeginRenderFrame();

	OwnerGameViewport->Draw();

	EnqueueEndRenderFrame();
}

void Viewport::EnqueueBeginRenderFrame()
{
	Viewport* viewport = this;
	RenderCommandList::Get().EnqueueLambdaCommand([viewport](RenderCommandList& CmdList)
	{
		viewport->BeginRenderFrame(CmdList);
	});
}

void Viewport::EnqueueEndRenderFrame()
{
	Viewport* viewport = this;
	RenderCommandList::Get().EnqueueLambdaCommand([viewport](RenderCommandList& CmdList)
	{
		viewport->EndRenderFrame(CmdList);
	});
}

void Viewport::BeginRenderFrame(RenderCommandList& CmdList)
{
	CmdList.BeginDrawingViewport(ViewportRHI);
	if (ViewportRHI.IsValid())
	{
		RenderTargetTextureRHI = RHI::RHIGetViewportBackBuffer(ViewportRHI);
	}
}

void Viewport::EndRenderFrame(RenderCommandList& CmdList)
{
	CmdList.EndDrawingViewport(GetViewportRHI());
}
}
