#include "Viewport.h"

#include "DynamicRHI.h"
#include "RenderCommandList.h"

namespace LE::Renderer
{
Viewport::Viewport(RefCountingPtr<RHI::RHIViewport> ViewportRHI)
	: ViewportRHI(ViewportRHI)
{}

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
