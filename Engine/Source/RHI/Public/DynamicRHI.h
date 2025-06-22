#pragma once

#include <span>

#include "CoreMinimum.h"

#include "RHIDefinitions.h"
#include "RHIResources.h"

namespace LE::RHI
{
class RHIContext;
}

namespace LE::Renderer
{
class RenderCommandList;
}

namespace LE::RHI
{
void InitRHI();
void DeleteRHI();

class DynamicRHI
{
public:
	virtual ~DynamicRHI()
	{
	}

	virtual void Init() = 0;

	virtual void Shutdown()
	{
	}

	virtual RHIInterfaceType GetInterfaceType() const { return RHIInterfaceType::None; }

	virtual RefCountingPtr<RHIConstantBuffer> RHICreateConstantBuffer(const void* Data, const RHIConstantBufferLayout* Layout)
	{
		return nullptr;
	}

	virtual void RHIUpdateConstantBuffer(Renderer::RenderCommandList& CmdList, RHIConstantBuffer* ConstantBuffer, const void* Data) = 0;

	virtual RefCountingPtr<RHIBuffer> RHICreateBuffer(const RHIBufferDesc& BufferDesc, RHIResourceCreateInfo& CreateInfo)
	{
		return nullptr;
	}

	virtual RefCountingPtr<RHIViewport> RHICreateViewport(void* WindowHandle, uint32 SizeX, uint32 SizeY, bool bIsFullscreen)
	{
		return nullptr;
	}

	virtual RefCountingPtr<RHIVertexShader> RHICreateVertexShader(std::span<const uint8> Code) { return nullptr; }
	virtual RefCountingPtr<RHIPixelShader> RHICreatePixelShader(std::span<const uint8> Code) { return nullptr; }
	virtual RefCountingPtr<RHITexture> RHICreateTexture(const RHITextureDesc& TextureDesc) { return nullptr; }

	virtual RefCountingPtr<RHIBoundShaderState> RHICreateBoundShaderState(RHIVertexShader* VertexShader, RHIPixelShader* PixelShader)
	{
		return nullptr;
	}

	virtual RefCountingPtr<RHIDepthStencilState> RHICreateDepthStencilState(const RHIDepthStencilStateDesc& DepthStencilStateDesc)
	{
		return nullptr;
	}

	virtual RefCountingPtr<RHISamplerState> RHICreateSamplerState(const RHISamplerStateInitializer& Initializer) { return nullptr; }

	virtual RefCountingPtr<RHIReadView> RHICreateReadView(Renderer::RenderCommandList& CmdList, RHIViewableResource* Resource,
	                                                      const RHIViewDescription& ViewDescription) = 0;
	virtual RefCountingPtr<RHIWriteView> RHICreateWriteView(Renderer::RenderCommandList& CmdList, RHIViewableResource* Resource,
	                                                        const RHIViewDescription& ViewDescription) = 0;

	virtual RefCountingPtr<RHITexture> RHIGetViewportBackBuffer(RHIViewport* Viewport) = 0;

	virtual void RHIBeginDrawingViewport(RHIViewport* Viewport) = 0;
	virtual void RHIEndDrawingViewport(RHIViewport* Viewport) = 0;

	virtual void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ) = 0;

	virtual void RHISetScissorRectangle(bool IsEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) = 0;

	virtual RHIContext* RHIGetContext() = 0;

	virtual RefCountingPtr<RHIPipelineStateObject> RHICreatePipelineStateObject(const PipelineStateInitializer& Initializer) = 0;
};

extern DynamicRHI* gDynamicRHI;

inline RHIInterfaceType GetInterfaceType()
{
	return gDynamicRHI->GetInterfaceType();
}

template <typename RHI>
inline RHI* GetDynamicRHI()
{
	return static_cast<RHI*>(gDynamicRHI);
}

inline RefCountingPtr<RHIConstantBufferLayout> RHICreateConstantBufferLayout(const RHIConstantBufferInitializer& Initializer)
{
	return new RHIConstantBufferLayout(Initializer);
}

inline RefCountingPtr<RHIConstantBuffer> RHICreateConstantBuffer(const void* Data, const RHIConstantBufferLayout* Layout)
{
	return gDynamicRHI->RHICreateConstantBuffer(Data, Layout);
}

inline RefCountingPtr<RHIVertexShader> RHICreateVertexShader(std::span<const uint8> Code)
{
	return gDynamicRHI->RHICreateVertexShader(Code);
}

inline RefCountingPtr<RHIPixelShader> RHICreatePixelShader(std::span<const uint8> Code)
{
	return gDynamicRHI->RHICreatePixelShader(Code);
}

inline RefCountingPtr<RHIDepthStencilState> RHICreateDepthStencilState(const RHIDepthStencilStateDesc& DepthStencilStateDesc)
{
	return gDynamicRHI->RHICreateDepthStencilState(DepthStencilStateDesc);
}

inline RefCountingPtr<RHISamplerState> RHICreateSamplerState(const RHISamplerStateInitializer& Initializer)
{
	return gDynamicRHI->RHICreateSamplerState(Initializer);
}

inline RefCountingPtr<RHITexture> RHIGetViewportBackBuffer(RHIViewport* Viewport)
{
	return gDynamicRHI->RHIGetViewportBackBuffer(Viewport);
}

inline RefCountingPtr<RHIViewport> RHICreateViewport(void* WindowHandle, uint32 SizeX, uint32 SizeY, bool IsFullScreen)
{
	return gDynamicRHI->RHICreateViewport(WindowHandle, SizeX, SizeY, IsFullScreen);
}

inline RefCountingPtr<RHIPipelineStateObject> RHICreatePipelineStateObject(const PipelineStateInitializer& Initializer)
{
	return gDynamicRHI->RHICreatePipelineStateObject(Initializer);
}

class DynamicRHIModule
{
public:
	virtual DynamicRHI* CreateRHI() = 0;
};

DynamicRHI* CreateDynamicRHI();
}
