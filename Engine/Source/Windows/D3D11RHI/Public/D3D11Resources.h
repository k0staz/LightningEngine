#pragma once

#include <d3d11.h>
#include <span>

#include "RenderCommandList.h"
#include "RHIResources.h"
#include "Containers/LinkedList.h"

namespace LE::D3D11
{
class D3D11View;
class D3D11ReadView;
class D3D11DynamicRHI;
}

namespace LE::D3D11
{
using namespace LE::RHI;

DXGI_FORMAT GetD3D11FormatFromPixelFormat(PixelFormat Format);

class D3D11ViewableResource
{
public:
	~D3D11ViewableResource()
	{
		LE_ASSERT(!HasLinkedViews())
	}

	bool HasLinkedViews() const
	{
		return LinkedViews != nullptr;
	}

	void UpdateLinkedViews();

protected:
	friend class D3D11ReadView;
	friend class D3D11WriteView;

	D3D11View* LinkedViews;
};

class D3D11Buffer : public RHIBuffer, public D3D11ViewableResource
{
public:
	D3D11Buffer(ID3D11Buffer* InResource, const RHIBufferDesc& InDesc)
		: RHIBuffer(InDesc)
		  , Resource(InResource)
	{
	}

	virtual ~D3D11Buffer();

	RefCountingPtr<ID3D11Buffer> GetResource() const { return Resource; }
	RefCountingPtr<ID3D11Buffer>& GetResource() { return Resource; }

private:
	RefCountingPtr<ID3D11Buffer> Resource;
};

class D3D11ConstantBuffer : public RHIConstantBuffer
{
public:
	D3D11ConstantBuffer(const RHIConstantBufferLayout* InLayout, ID3D11Buffer* InResource)
		: RHIConstantBuffer(InLayout)
		  , Resource(InResource)
	{
	}

	RefCountingPtr<ID3D11Buffer> GetResource() const { return Resource; }
	RefCountingPtr<ID3D11Buffer>& GetResource() { return Resource; }

private:
	RefCountingPtr<ID3D11Buffer> Resource;
};

class D3D11Texture : public RHITexture, public D3D11ViewableResource
{
public:
	D3D11Texture(const RHITextureDesc& InDesc, ID3D11Resource* InResource, ID3D11ShaderResourceView* InShaderResource,
	             std::span<RefCountingPtr<ID3D11RenderTargetView>> InRenderTargetViews,
	             std::span<RefCountingPtr<ID3D11DepthStencilView>> InDepthStencilViews);
	~D3D11Texture();

	RefCountingPtr<ID3D11Resource> GetResource() const { return Resource; }
	RefCountingPtr<ID3D11Resource>& GetResource() { return Resource; }
	RefCountingPtr<ID3D11ShaderResourceView> GetShaderResourceView() const { return ShaderResourceView; }
	RefCountingPtr<ID3D11ShaderResourceView>& GetShaderResourceView() { return ShaderResourceView; }

	ID3D11Texture2D* GetD3D11Texture2D() const
	{
		LE_ASSERT(GetDescription().Dimension == TextureDimensions::Texture2D)

		return static_cast<ID3D11Texture2D*>(Resource.GetPointer());
	}

	ID3D11Texture3D* GetD3D11Texture3D() const
	{
		LE_ASSERT(GetDescription().Dimension == TextureDimensions::Texture3D)

		return static_cast<ID3D11Texture3D*>(Resource.GetPointer());
	}

	bool IsTexture3D() const { return GetDescription().Dimension == TextureDimensions::Texture3D; }

	RefCountingPtr<ID3D11RenderTargetView> GetRenderTargetView(const uint32 MipIndex) const
	{
		if (MipIndex < RenderTargetViews.size())
		{
			return RenderTargetViews[MipIndex];
		}

		return nullptr;
	}

	RefCountingPtr<ID3D11DepthStencilView> GetDepthStencilView(const ExclusiveDepthStencil AccessType) const
	{
		return DepthStencilViews[AccessType.GetIndex()];
	}

private:
	RefCountingPtr<ID3D11Resource> Resource;
	RefCountingPtr<ID3D11ShaderResourceView> ShaderResourceView;
	std::vector<RefCountingPtr<ID3D11RenderTargetView>> RenderTargetViews;
	RefCountingPtr<ID3D11DepthStencilView> DepthStencilViews[ExclusiveDepthStencil::MaxIndex];
};

class D3D11DepthStencilState : public RHIDepthStencilState
{
public:
	RefCountingPtr<ID3D11DepthStencilState> GetResource() const { return Resource; }
	RefCountingPtr<ID3D11DepthStencilState>& GetResource() { return Resource; }

	ExclusiveDepthStencil& GetAccessType() { return AccessType; }
	const ExclusiveDepthStencil& GetAccessType() const { return AccessType; }

private:
	RefCountingPtr<ID3D11DepthStencilState> Resource;
	ExclusiveDepthStencil AccessType;
};

class D3D11VertexShader : public RHIVertexShader
{
public:
	RefCountingPtr<ID3D11VertexShader> GetResource() const { return Resource; }
	RefCountingPtr<ID3D11VertexShader>& GetResource() { return Resource; }

private:
	RefCountingPtr<ID3D11VertexShader> Resource;
};

class D3D11PixelShader : public RHIPixelShader
{
public:
	RefCountingPtr<ID3D11PixelShader> GetResource() const { return Resource; }
	RefCountingPtr<ID3D11PixelShader>& GetResource() { return Resource; }

private:
	RefCountingPtr<ID3D11PixelShader> Resource;
};

class D3D11SamplerState : public RHISamplerState
{
public:
	RefCountingPtr<ID3D11SamplerState> GetResource() const { return Resource; }
	RefCountingPtr<ID3D11SamplerState>& GetResource() { return Resource; }
private:
	RefCountingPtr<ID3D11SamplerState> Resource;
};

class D3D11View : public IntrusiveLinkedList<D3D11View>
{
public:
	virtual ~D3D11View();
	virtual void Update() = 0;
};

class D3D11ReadView : public RHIReadView, public D3D11View
{
public:
	D3D11ReadView(Renderer::RenderCommandList& CmdList, RHIViewableResource* InResource, const RHIViewDescription& InViewDescription);

	D3D11ViewableResource* GetBaseResource() const;
	RefCountingPtr<ID3D11ShaderResourceView> GetView() const { return View; }

	virtual void Update() override;

private:
	RefCountingPtr<ID3D11ShaderResourceView> View;
};

class D3D11WriteView: public RHIWriteView, public D3D11View
{
public:
	D3D11WriteView(Renderer::RenderCommandList& CmdList, RHIViewableResource* InResource, const RHIViewDescription& InViewDescription);

	D3D11ViewableResource* GetBaseResource() const;
	RefCountingPtr<ID3D11UnorderedAccessView> GetView() const { return View; }

	virtual void Update() override;

private:
	RefCountingPtr<ID3D11UnorderedAccessView> View;
};

class D3D11BoundShaderState : public RHIBoundShaderState
{
public:
	D3D11BoundShaderState(RHIVertexShader* InVertexShader, RHIPixelShader* InPixelShader);

	RefCountingPtr<RHIVertexShader> VertexShaderRHI;
	RefCountingPtr<RHIPixelShader> PixelShaderRHI;
	RefCountingPtr<ID3D11VertexShader> VertexShader;
	RefCountingPtr<ID3D11PixelShader> PixelShader;
};

template<class T>
struct D3D11ResourceCaster
{
};

template<>
struct D3D11ResourceCaster<RHIBuffer>
{
	using D3D11Type = D3D11Buffer;
};

template<>
struct D3D11ResourceCaster<RHITexture>
{
	using D3D11Type = D3D11Texture;
};

template<>
struct D3D11ResourceCaster<RHIConstantBuffer>
{
	using D3D11Type = D3D11ConstantBuffer;
};

template<>
struct D3D11ResourceCaster<RHIViewport>
{
	using D3D11Type = class D3D11Viewport;
};

template<>
struct D3D11ResourceCaster<RHIVertexShader>
{
	using D3D11Type = D3D11VertexShader;
};

template<>
struct D3D11ResourceCaster<RHIPixelShader>
{
	using D3D11Type = D3D11PixelShader;
};

template<>
struct D3D11ResourceCaster<RHIBoundShaderState>
{
	using D3D11Type = D3D11BoundShaderState;
};

template<>
struct D3D11ResourceCaster<RHIDepthStencilState>
{
	using D3D11Type = D3D11DepthStencilState;
};

template<>
struct D3D11ResourceCaster<RHIReadView>
{
	using D3D11Type = D3D11ReadView;
};

template<>
struct D3D11ResourceCaster<RHIWriteView>
{
	using D3D11Type = D3D11WriteView;
};

template<>
struct D3D11ResourceCaster<RHISamplerState>
{
	using D3D11Type = D3D11SamplerState;
};

template<typename RHIType>
static typename D3D11ResourceCaster<RHIType>::D3D11Type* ResourceCast(RHIType* Resource)
{
	return static_cast<typename D3D11ResourceCaster<RHIType>::D3D11Type*>(Resource);
}
}
