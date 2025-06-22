#pragma once
#include "CoreDefinitions.h"
#include "DynamicRHI.h"
#include "RenderResource.h"


namespace LE::RHI
{
template <typename InitializerType, typename RHIRefCountedType, typename RHIParamType>
class StaticStateRHI
{
public:
	static RHIParamType GetRHI()
	{
		return StaticResource.StateRHI;
	}

private:
	class StaticStateResource : public Renderer::RenderResource
	{
	public:
		StaticStateResource()
		{
			Renderer::RenderCommandList::Get().EnqueueLambdaCommand([this](Renderer::RenderCommandList& CmdList)
			{
				InitRHI(CmdList);
			});
		}

		virtual void InitRHI(Renderer::RenderCommandList& CommandList) override
		{
			StateRHI = InitializerType::CreateRHI();
		}


		RHIRefCountedType StateRHI;
	};

	static StaticStateResource StaticResource;
};

template <typename InitializerType, typename RHIRefCountedType, typename RHIParamType>
typename StaticStateRHI<InitializerType, RHIRefCountedType, RHIParamType>::StaticStateResource StaticStateRHI<
	InitializerType, RHIRefCountedType, RHIParamType>::StaticResource = StaticStateRHI<
	InitializerType, RHIRefCountedType, RHIParamType>::StaticStateResource();

template <bool bEnableDepthWrite = true,
          CompareFunction DepthTest = CompareFunction::GreaterEqual,
          bool bEnableFrontFaceStencil = false,
          CompareFunction FrontFaceStencilTest = CompareFunction::Always,
          StencilOp FrontFaceStencilFailStencilOp = StencilOp::Keep,
          StencilOp FrontFaceDepthFailStencilOp = StencilOp::Keep,
          StencilOp FrontFaceStencilPassStencilOp = StencilOp::Keep,
          bool bEnableBackFaceStencil = false,
          CompareFunction BackFaceStencilTest = CompareFunction::Always,
          StencilOp BackFaceStencilFailStencilOp = StencilOp::Keep,
          StencilOp BackFaceDepthFailStencilOp = StencilOp::Keep,
          StencilOp BackFaceStencilPassStencilOp = StencilOp::Keep,
          uint8 StencilReadMask = 0xFF,
          uint8 StencilWriteMask = 0xFF>
class StaticDepthStencilState : public StaticStateRHI<
		StaticDepthStencilState<
			bEnableDepthWrite,
			DepthTest,
			bEnableFrontFaceStencil,
			FrontFaceStencilTest,
			FrontFaceStencilFailStencilOp,
			FrontFaceDepthFailStencilOp,
			FrontFaceStencilPassStencilOp,
			bEnableBackFaceStencil,
			BackFaceStencilTest,
			BackFaceStencilFailStencilOp,
			BackFaceDepthFailStencilOp,
			BackFaceStencilPassStencilOp,
			StencilReadMask,
			StencilWriteMask
		>,
		RefCountingPtr<RHIDepthStencilState>
		, RHIDepthStencilState*>
{
public:
	static RefCountingPtr<RHIDepthStencilState> CreateRHI()
	{
		RHIDepthStencilStateDesc depthStencilStateDesc(
			bEnableDepthWrite,
			DepthTest,
			bEnableFrontFaceStencil,
			FrontFaceStencilTest,
			FrontFaceStencilFailStencilOp,
			FrontFaceDepthFailStencilOp,
			FrontFaceStencilPassStencilOp,
			bEnableBackFaceStencil,
			BackFaceStencilTest,
			BackFaceStencilFailStencilOp,
			BackFaceDepthFailStencilOp,
			BackFaceStencilPassStencilOp,
			StencilReadMask,
			StencilWriteMask);

		return RHICreateDepthStencilState(depthStencilStateDesc);
	}
};
}
