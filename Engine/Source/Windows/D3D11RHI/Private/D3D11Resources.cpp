#include "D3D11Resources.h"

#include "D3D11DynamicRHI.h"

namespace LE::D3D11
{
DXGI_FORMAT GetD3D11FormatFromPixelFormat(PixelFormat Format)
{
	switch (Format)
	{
	case PixelFormat::R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case PixelFormat::R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case PixelFormat::R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case PixelFormat::Invalid:
	case PixelFormat::Count:
	default:
		{
		LE_ERROR("Unhandled Pixel Format");
		return DXGI_FORMAT_UNKNOWN;
		}
	}
}

void D3D11ViewableResource::UpdateLinkedViews()
{
	for (D3D11View* view = LinkedViews; view; view = view->GetNextElement())
	{
		view->Update();
	}
}

D3D11Buffer::~D3D11Buffer()
{
}

D3D11Texture::D3D11Texture(const RHITextureDesc& InDesc, ID3D11Resource* InResource, ID3D11ShaderResourceView* InShaderResource,
                           std::span<RefCountingPtr<ID3D11RenderTargetView>> InRenderTargetViews,
                           std::span<RefCountingPtr<ID3D11DepthStencilView>> InDepthStencilViews)
	: RHITexture(InDesc)
	  , Resource(InResource)
	  , ShaderResourceView(InShaderResource)
	  , RenderTargetViews(InRenderTargetViews.begin(), InRenderTargetViews.end())

{
	if (InDepthStencilViews.size())
	{
		for (uint32 index = 0; index < ExclusiveDepthStencil::MaxIndex; ++index)
		{
			DepthStencilViews[index] = InDepthStencilViews[index];
		}
	}
}

D3D11Texture::~D3D11Texture()
{
}

D3D11View::~D3D11View()
{
	Unlink();
}

D3D11ReadView::D3D11ReadView(Renderer::RenderCommandList& CmdList, RHIViewableResource* InResource,
                             const RHIViewDescription& InViewDescription)
	: RHIReadView(InViewDescription, InResource)
{
	CmdList.EnqueueLambdaCommand([this](Renderer::RenderCommandList&)
	{
		LinkAsHead(GetBaseResource()->LinkedViews);
		Update();
	});
}

D3D11ViewableResource* D3D11ReadView::GetBaseResource() const
{
	return IsBufferView()
		       ? static_cast<D3D11ViewableResource*>(ResourceCast(GetBuffer()))
		       : static_cast<D3D11ViewableResource*>(ResourceCast(GetTexture()));
}

void D3D11ReadView::Update()
{
	ID3D11Resource* d3dResource = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};

	if (IsBufferView())
	{
		D3D11Buffer* buffer = ResourceCast(GetBuffer());
		const auto info = ViewDescription.Buffer.ReadView.GetViewInfo(buffer);

		if (!info.IsNull)
		{
			d3dResource = buffer->GetResource();
			desc.Format = GetD3D11FormatFromPixelFormat(ViewDescription.Buffer.ReadView.Format);

			switch (info.Type)
			{
			case RHIViewDescription::BufferType::Typed:
			case RHIViewDescription::BufferType::Structured:
				desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				desc.Buffer.FirstElement = info.OffsetInBytes / info.StrideInBytes;
				desc.Buffer.NumElements = info.ElementsNum;
				break;
			case RHIViewDescription::BufferType::Raw:
				desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
				desc.Format = DXGI_FORMAT_R32_TYPELESS;
				desc.BufferEx.FirstElement = info.OffsetInBytes / info.StrideInBytes;
				desc.BufferEx.NumElements = info.ElementsNum;
				desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		D3D11Texture* texture = ResourceCast(GetTexture());
		d3dResource = texture->GetResource();

		const RHITextureDesc& textureDesc = texture->GetDescription();
		auto const info = ViewDescription.Texture.ReadView.GetViewInfo(texture);

		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		switch (textureDesc.Dimension)
		{
		case TextureDimensions::Texture2D:
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.MipLevels = 1;
			break;
		case TextureDimensions::Texture3D:
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MostDetailedMip = 0;
			desc.Texture3D.MipLevels = 1;
			break;
		}
	}

	View = nullptr;
	if (d3dResource)
	{
		ID3D11Device* device = D3D11DynamicRHI::Get().GetDevice();
		VERIFYD3D11RESULT(device->CreateShaderResourceView(d3dResource, &desc, View.GetInitPointer()))
	}
}

D3D11WriteView::D3D11WriteView(Renderer::RenderCommandList& CmdList, RHIViewableResource* InResource,
                               const RHIViewDescription& InViewDescription)
	: RHIWriteView(InViewDescription, InResource)
{
	CmdList.EnqueueLambdaCommand([this](Renderer::RenderCommandList&)
	{
		LinkAsHead(GetBaseResource()->LinkedViews);
		Update();
	});
}

D3D11ViewableResource* D3D11WriteView::GetBaseResource() const
{
	return IsBufferView()
		       ? static_cast<D3D11ViewableResource*>(ResourceCast(GetBuffer()))
		       : static_cast<D3D11ViewableResource*>(ResourceCast(GetTexture()));
}

void D3D11WriteView::Update()
{
	ID3D11Resource* d3dResource = nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};

	if (IsBufferView())
	{
		D3D11Buffer* buffer = ResourceCast(GetBuffer());
		const auto info = ViewDescription.Buffer.WriteView.GetViewInfo(buffer);

		if (!info.IsNull)
		{
			d3dResource = buffer->GetResource();
			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

			desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

			desc.Buffer.FirstElement = info.OffsetInBytes / info.StrideInBytes;
			desc.Buffer.NumElements = info.ElementsNum;

			switch (info.Type)
			{
			case RHIViewDescription::BufferType::Typed:
			case RHIViewDescription::BufferType::Structured:
				break;
			case RHIViewDescription::BufferType::Raw:
				desc.Format = DXGI_FORMAT_R32_TYPELESS;
				desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		D3D11Texture* texture = ResourceCast(GetTexture());
		d3dResource = texture->GetResource();

		const RHITextureDesc& textureDesc = texture->GetDescription();
		auto const info = ViewDescription.Texture.ReadView.GetViewInfo(texture);

		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		switch (textureDesc.Dimension)
		{
		case TextureDimensions::Texture2D:
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = 1;
			break;
		case TextureDimensions::Texture3D:
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
			desc.Texture3D.FirstWSlice = 0;
			desc.Texture3D.WSize = 1;
			desc.Texture3D.MipSlice = 1;
			break;
		}
	}

	View = nullptr;
	if (d3dResource)
	{
		ID3D11Device* device = D3D11DynamicRHI::Get().GetDevice();
		VERIFYD3D11RESULT(device->CreateUnorderedAccessView(d3dResource, &desc, View.GetInitPointer()))
	}
}

D3D11BoundShaderState::D3D11BoundShaderState(RHIVertexShader* InVertexShader, RHIPixelShader* InPixelShader)
{
	VertexShaderRHI = InVertexShader;
	PixelShaderRHI = InPixelShader;

	VertexShader = ResourceCast(InVertexShader)->GetResource();
	PixelShader = ResourceCast(InPixelShader)->GetResource();
}
}
