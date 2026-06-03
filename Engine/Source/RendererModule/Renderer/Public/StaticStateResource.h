#pragma once

#include "DynamicRHI.h"
#include "RenderResource.h"

namespace LE::Renderer
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
          RHI::CompareFunction DepthTest = RHI::CompareFunction::GreaterEqual,
          bool bEnableFrontFaceStencil = false,
          RHI::CompareFunction FrontFaceStencilTest = RHI::CompareFunction::Always,
          RHI::StencilOp FrontFaceStencilFailStencilOp = RHI::StencilOp::Keep,
          RHI::StencilOp FrontFaceDepthFailStencilOp = RHI::StencilOp::Keep,
          RHI::StencilOp FrontFaceStencilPassStencilOp = RHI::StencilOp::Keep,
          bool bEnableBackFaceStencil = false,
          RHI::CompareFunction BackFaceStencilTest = RHI::CompareFunction::Always,
          RHI::StencilOp BackFaceStencilFailStencilOp = RHI::StencilOp::Keep,
          RHI::StencilOp BackFaceDepthFailStencilOp = RHI::StencilOp::Keep,
          RHI::StencilOp BackFaceStencilPassStencilOp = RHI::StencilOp::Keep,
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
		RefCountingPtr<RHI::RHIDepthStencilState>
		, RHI::RHIDepthStencilState*>
{
public:
	static RefCountingPtr<RHI::RHIDepthStencilState> CreateRHI()
	{
		RHI::RHIDepthStencilStateDesc depthStencilStateDesc(
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
