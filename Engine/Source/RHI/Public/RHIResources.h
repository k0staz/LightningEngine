#pragma once

#include "CoreMinimum.h"
#include <limits>
#include <utility>

#include "RHIDefinitions.h"
#include "Containers/Array.h"
#include "Containers/ResourceArrays.h"
#include "Math/LinearColor.h"

#undef max

namespace LE::Renderer
{
class RenderCommandList;
}

namespace LE::RHI
{
struct RHIConstantBufferResourceInitializer;
struct RHIConstantBufferInitializer;

class RHIResource : public RefCountableBase
{
public:
	RHIResource(RHIResourceType InResourceType)
		: ResourceType(InResourceType)
	{
	}

	RHIResource(const RHIResource&) = delete;
	RHIResource& operator=(const RHIResource&) = delete;

	virtual ~RHIResource() = default;

	inline RHIResourceType GetResourceType() const { return ResourceType; }

protected:
	RHIResourceType ResourceType;
};

class RHIViewport : public RHIResource
{
public:
	RHIViewport() : RHIResource(RHIResourceType::Viewport)
	{
	}
};

struct RHIBufferDesc
{
	RHIBufferDesc() = default;

	RHIBufferDesc(uint32 InSize, uint32 InStride, BufferUsageFlags InUsage)
		: Size(InSize)
		  , Stride(InStride)
		  , Usage(InUsage)
	{
	}

	static RHIBufferDesc Null()
	{
		return RHIBufferDesc(0, 0, BUF_None);
	}

	bool IsNull() const
	{
		return Usage == BUF_None && Size == 0 && Stride == 0;
	}

	uint32 Size = 0;
	uint32 Stride = 0;
	BufferUsageFlags Usage = BUF_None;
};

struct RHIResourceCreateInfo
{
	RHIResourceCreateInfo()
		: ResourceArray(nullptr)
	{
	}

	RHIResourceCreateInfo(Renderer::ResourceArrayInterface* InResourceArray)
		: ResourceArray(InResourceArray)
	{
	}

	Renderer::ResourceArrayInterface* ResourceArray;
};

class RHIViewableResource : public RHIResource
{
public:
	RHIViewableResource(RHIResourceType Type)
		: RHIResource(Type)
	{
	}
};

class RHIBuffer : public RHIViewableResource
{
public:
	RHIBuffer(const RHIBufferDesc& InDesc) : RHIViewableResource(RHIResourceType::Buffer), Description(InDesc)
	{
	}

	RHIBufferDesc const& GetDescription() const { return Description; }

	uint32 GetSize() const { return Description.Size; }
	uint32 GetStride() const { return Description.Stride; }
	BufferUsageFlags GetUsage() const { return Description.Usage; }

private:
	RHIBufferDesc Description;
};

struct RHITextureDesc
{
	RHITextureDesc() = default;

	uint32 SizeX;
	uint32 SizeY;
	uint8 NumMips;
	TextureCreateFlags CreateFlags = TCF_None;
	TextureDimensions Dimension = TextureDimensions::Texture2D;
};

class RHITexture : public RHIViewableResource
{
public:
	RHITexture(const RHITextureDesc& InTextureDesc) : RHIViewableResource(RHIResourceType::Texture), Description(InTextureDesc)
	{
	}

	virtual const RHITextureDesc& GetDescription() const { return Description; }

private:
	RHITextureDesc Description;
};

class RHIShader : public RHIResource
{
public:
	RHIShader() = delete;

	RHIShader(RHIResourceType InResourceType, ShaderType InShaderType)
		: RHIResource(InResourceType)
		  , Type(InShaderType)
	{
	}

	ShaderType GetShaderType() const { return Type; }

private:
	ShaderType Type;
};

class RHIVertexShader : public RHIShader
{
public:
	RHIVertexShader() : RHIShader(RHIResourceType::VertexShader, ShaderType::Vertex)
	{
	}
};

class RHIPixelShader : public RHIShader
{
public:
	RHIPixelShader() : RHIShader(RHIResourceType::PixelShader, ShaderType::Pixel)
	{
	}
};

struct RHIDepthStencilStateDesc
{
	RHIDepthStencilStateDesc(
		bool InEnableDepthWrite = true,
		CompareFunction InDepthTest = CompareFunction::LessEqual,
		bool InEnableFrontFaceStencil = false,
		CompareFunction InFrontFaceStencilTest = CompareFunction::Always,
		StencilOp InFrontFaceStencilFailOp = StencilOp::Keep,
		StencilOp InFrontFaceDepthFailOp = StencilOp::Keep,
		StencilOp InFrontFacePassFailOp = StencilOp::Keep,
		bool InEnableBackFaceStencil = false,
		CompareFunction InBackFaceStencilTest = CompareFunction::Always,
		StencilOp InBackFaceStencilFailOp = StencilOp::Keep,
		StencilOp InBackFaceDepthFailOp = StencilOp::Keep,
		StencilOp InBackFacePassFailOp = StencilOp::Keep,
		uint8 InStencilReadMask = 0xFF,
		uint8 InStencilWriteMask = 0xFF
	)
		: bEnableDepthWrite(InEnableDepthWrite)
		  , DepthTest(InDepthTest)
		  , bEnableFrontFaceStencil(InEnableFrontFaceStencil)
		  , FrontFaceStencilTest(InFrontFaceStencilTest)
		  , FrontFaceStencilFailOp(InFrontFaceStencilFailOp)
		  , FrontFaceDepthFailOp(InFrontFaceDepthFailOp)
		  , FrontFacePassFailOp(InFrontFacePassFailOp)
		  , bEnableBackFaceStencil(InEnableBackFaceStencil)
		  , BackFaceStencilTest(InBackFaceStencilTest)
		  , BackFaceStencilFailOp(InBackFaceStencilFailOp)
		  , BackFaceDepthFailOp(InBackFaceDepthFailOp)
		  , BackFacePassFailOp(InBackFacePassFailOp)
		  , StencilReadMask(InStencilReadMask)
		  , StencilWriteMask(InStencilWriteMask)
	{
	}

	bool bEnableDepthWrite;
	CompareFunction DepthTest;

	bool bEnableFrontFaceStencil;
	CompareFunction FrontFaceStencilTest;
	StencilOp FrontFaceStencilFailOp;
	StencilOp FrontFaceDepthFailOp;
	StencilOp FrontFacePassFailOp;

	bool bEnableBackFaceStencil;
	CompareFunction BackFaceStencilTest;
	StencilOp BackFaceStencilFailOp;
	StencilOp BackFaceDepthFailOp;
	StencilOp BackFacePassFailOp;

	uint8 StencilReadMask;
	uint8 StencilWriteMask;
};

class RHIDepthStencilState : public RHIResource
{
public:
	RHIDepthStencilState() : RHIResource(RHIResourceType::DepthStencilState)
	{
	}
};

class ExclusiveDepthStencil
{
public:
	enum Type : uint8
	{
		DepthNop = 0x00,
		DepthRead = 0x01,
		DepthWrite = 0x02,
		DepthMask = 0x0f,

		StencilNop = 0x00,
		StencilRead = 0x10,
		StencilWrite = 0x20,
		StencilMask = 0xf0,

		DepthNop_StencilNop = DepthNop + StencilNop,
		DepthRead_StencilNop = DepthRead + StencilNop,
		DepthWrite_StencilNop = DepthWrite + StencilNop,

		DepthNop_StencilRead = DepthNop + StencilRead,
		DepthRead_StencilRead = DepthRead + StencilRead,
		DepthWrite_StencilRead = DepthWrite + StencilRead,

		DepthNop_StencilWrite = DepthNop + StencilWrite,
		DepthRead_StencilWrite = DepthRead + StencilWrite,
		DepthWrite_StencilWrite = DepthWrite + StencilWrite,
	};

	ExclusiveDepthStencil(Type InValue = DepthNop_StencilNop)
		: Value(InValue)
	{
	}

	void SetDepthStencilWrite(bool Depth, bool Stencil)
	{
		Value = DepthNop_StencilNop;
		if (Depth)
		{
			Value = static_cast<Type>(GetStencil() | DepthWrite);
		}

		if (Stencil)
		{
			Value = static_cast<Type>(GetDepth() | StencilWrite);
		}
	}

	Type GetDepth() const
	{
		return static_cast<Type>(Value & DepthMask);
	}

	Type GetStencil() const
	{
		return static_cast<Type>(Value & StencilMask);
	}

	uint8 GetIndex() const
	{
		switch (Value)
		{
		case DepthWrite_StencilNop:
		case DepthNop_StencilWrite:
		case DepthWrite_StencilWrite:
		case DepthNop_StencilNop:
			return 0;

		case DepthRead_StencilNop:
		case DepthRead_StencilWrite:
			return 1;

		case DepthNop_StencilRead:
		case DepthWrite_StencilRead:
			return 2;

		case DepthRead_StencilRead:
			return 3;
		}

		LE_ASSERT(false)
		return -1;
	}

	bool operator==(const ExclusiveDepthStencil& Other) const
	{
		return Value == Other.Value;
	}

	static constexpr uint32 MaxIndex = 4;

protected:
	Type Value;
};

struct RHIConstantBufferResource
{
	RHIConstantBufferResource() = delete;
	RHIConstantBufferResource(const RHIConstantBufferResourceInitializer& Initializer);

	friend bool operator==(const RHIConstantBufferResource& Left, const RHIConstantBufferResource& Right)
	{
		return Left.ResourceOffset == Right.ResourceOffset && Left.ResourceType == Right.ResourceType;
	}

	String Name;
	uint16 ResourceOffset;
	ShaderParameterType ResourceType;
};

struct RHIConstantBufferLayout : public RHIResource
{
	RHIConstantBufferLayout() = delete;
	RHIConstantBufferLayout(const RHIConstantBufferInitializer& Initializer);

	const String Name;

	const Array<RHIConstantBufferResource> Resources;

	const uint32 ConstantBufferSize;

	friend bool operator==(const RHIConstantBufferLayout& Left, const RHIConstantBufferLayout& Right)
	{
		return Left.ConstantBufferSize == Right.ConstantBufferSize
			&& Left.Resources == Right.Resources;
	}
};

class RHIConstantBuffer : public RHIResource
{
public:
	RHIConstantBuffer() = delete;

	RHIConstantBuffer(const RHIConstantBufferLayout* InLayout)
		: RHIResource(RHIResourceType::ConstantBuffer)
		  , Layout(InLayout)
		  , LayoutConstantBufferSize(InLayout->ConstantBufferSize)
	{
	}

	uint32 GetSize() const
	{
		return LayoutConstantBufferSize;
	}

	const RHIConstantBufferLayout& GetLayout() const { return *Layout; }
	const RHIConstantBufferLayout* GetLayoutPtr() const { return Layout; }

	const Array<RefCountingPtr<RHIResource>>& GetResources() const { return Resources; }

protected:
	Array<RefCountingPtr<RHIResource>> Resources;

private:
	RefCountingPtr<const RHIConstantBufferLayout> Layout;
	uint32 LayoutConstantBufferSize;
};

class RHIVertexBufferLayout : public RHIResource
{
public:
	RHIVertexBufferLayout() : RHIResource(RHIResourceType::VertexBufferLayout)
	{
	}
};

struct RHISamplerStateInitializer
{
	SamplerFilter Filter = SamplerFilter::Point;
	SamplerAddressMode AddressU = SamplerAddressMode::Wrap;
	SamplerAddressMode AddressV = SamplerAddressMode::Wrap;
	SamplerAddressMode AddressW = SamplerAddressMode::Wrap;
	float MipMapBias = 0.0f;
	float MinMipMapLevel = 0.0f;
	float MaxMipMapLevel = FLOAT_MAX;
	int32 MaxAnisotropy = 0;
	LinearColor BorderColor = LinearColor::Black();
	SamplerCompareFunction SamplerCompareFunction = SamplerCompareFunction::Never;
};

class RHISamplerState : public RHIResource
{
public:
	RHISamplerState() : RHIResource(RHIResourceType::SamplerState)
	{
	}
};

struct RHIViewDescription
{
	RHIViewDescription()
	{
		MemsetZero(this, sizeof(*this));
		Common.Type = ViewType::BufferReadView;
	}

	enum class ViewType : uint8
	{
		BufferReadView,
		BufferWriteView,
		TextureReadView,
		TextureWriteView,
	};

	enum class BufferType : uint8
	{
		Invalid,

		Typed,
		Structured,
		Raw,
	};

	struct CommonInfo
	{
		ViewType Type;
		PixelFormat Format;
	};

	struct BufferInfo : public CommonInfo
	{
		BufferType TypeBuffer;
		uint32 OffsetInBytes = 0;
		uint32 ElementsNum = 0;
		uint32 Stride = 0;

		struct ViewInfo;

	protected:
		ViewInfo GetViewInfo(const RHIBuffer* Buffer) const;
	};

	struct BufferReadViewInfo : public BufferInfo
	{
		struct Initializer;
		struct ViewInfo;
		ViewInfo GetViewInfo(const RHIBuffer* Buffer) const;
	};

	struct BufferWriteViewInfo : public BufferInfo
	{
		struct Initializer;
		struct ViewInfo;
		ViewInfo GetViewInfo(const RHIBuffer* Buffer) const;
	};

	struct TextureInfo : public CommonInfo
	{
		TextureDimensions Dimensions;

		struct ViewInfo;

	protected:
		ViewInfo GetViewInfo(const RHITexture* Texture) const;
	};

	struct TextureReadViewInfo : public TextureInfo
	{
		struct Initializer;
		struct ViewInfo;
		ViewInfo GetViewInfo(const RHITexture* Texture) const;
	};

	struct TextureWriteViewInfo : public TextureInfo
	{
		struct Initializer;
		struct ViewInfo;
		ViewInfo GetViewInfo(const RHITexture* Texture) const;
	};

	union
	{
		CommonInfo Common;

		union
		{
			BufferReadViewInfo ReadView;
			BufferWriteViewInfo WriteView;
		} Buffer;

		union
		{
			TextureReadViewInfo ReadView;
			TextureWriteViewInfo WriteView;
		} Texture;
	};

	static inline BufferReadViewInfo::Initializer CreateBufferReadView();
	static inline BufferWriteViewInfo::Initializer CreateBufferWriteView();

	static inline TextureReadViewInfo::Initializer CreateTextureReadView();
	static inline TextureWriteViewInfo::Initializer CreateTextureWriteView();

	bool IsReadView() const { return Common.Type == ViewType::BufferReadView || Common.Type == ViewType::TextureReadView; }
	bool IsWriteView() const { return Common.Type == ViewType::BufferWriteView || Common.Type == ViewType::TextureWriteView; }

	bool IsBufferView() const { return Common.Type == ViewType::BufferReadView || Common.Type == ViewType::BufferWriteView; }
	bool IsTextureView() const { return !IsBufferView(); }

protected:
	RHIViewDescription(ViewType Type)
	{
		MemsetZero(this, sizeof(*this));
		Common.Type = Type;
	}
};

struct RHIViewDescription::BufferReadViewInfo::Initializer : private RHIViewDescription
{
	friend RHIViewDescription;
	friend Renderer::RenderCommandList;

protected:
	Initializer()
		: RHIViewDescription(ViewType::BufferReadView)
	{
	}

public:
	Initializer& SetType(BufferType BufferType)
	{
		Buffer.ReadView.TypeBuffer = BufferType;
		return *this;
	}

	Initializer& SetFormat(PixelFormat Format)
	{
		Buffer.ReadView.Format = Format;
		return *this;
	}

	Initializer& SetOffsetInBytes(uint32 OffsetInBytes)
	{
		Buffer.ReadView.OffsetInBytes = OffsetInBytes;
		return *this;
	}

	Initializer& SetElementsNum(uint32 ElementsNum)
	{
		Buffer.ReadView.ElementsNum = ElementsNum;
		return *this;
	}

	Initializer& SetStride(uint32 Stride)
	{
		Buffer.ReadView.Stride = Stride;
		return *this;
	}
};

struct RHIViewDescription::BufferWriteViewInfo::Initializer : private RHIViewDescription
{
	friend RHIViewDescription;
	friend Renderer::RenderCommandList;

protected:
	Initializer()
		: RHIViewDescription(ViewType::BufferWriteView)
	{
	}

public:
	Initializer& SetType(BufferType BufferType)
	{
		Buffer.WriteView.TypeBuffer = BufferType;
		return *this;
	}

	Initializer& SetOffsetInBytes(uint32 OffsetInBytes)
	{
		Buffer.WriteView.OffsetInBytes = OffsetInBytes;
		return *this;
	}

	Initializer& SetElementsNum(uint32 ElementsNum)
	{
		Buffer.WriteView.ElementsNum = ElementsNum;
		return *this;
	}

	Initializer& SetStride(uint32 Stride)
	{
		Buffer.WriteView.Stride = Stride;
		return *this;
	}
};

struct RHIViewDescription::TextureReadViewInfo::Initializer : private RHIViewDescription
{
	friend RHIViewDescription;
	friend Renderer::RenderCommandList;

protected:
	Initializer()
		: RHIViewDescription(ViewType::TextureReadView)
	{
	}

public:
	Initializer& SetTextureDimensions(TextureDimensions Dimensions)
	{
		Texture.ReadView.Dimensions = Dimensions;
		return *this;
	}
};

struct RHIViewDescription::TextureWriteViewInfo::Initializer : private RHIViewDescription
{
	friend RHIViewDescription;
	friend Renderer::RenderCommandList;

protected:
	Initializer()
		: RHIViewDescription(ViewType::TextureWriteView)
	{
	}

public:
	Initializer& SetTextureDimensions(TextureDimensions Dimensions)
	{
		Texture.WriteView.Dimensions = Dimensions;
		return *this;
	}
};

inline RHIViewDescription::BufferReadViewInfo::Initializer RHIViewDescription::CreateBufferReadView()
{
	return {};
}

inline RHIViewDescription::BufferWriteViewInfo::Initializer RHIViewDescription::CreateBufferWriteView()
{
	return {};
}

inline RHIViewDescription::TextureReadViewInfo::Initializer RHIViewDescription::CreateTextureReadView()
{
	return {};
}

inline RHIViewDescription::TextureWriteViewInfo::Initializer RHIViewDescription::CreateTextureWriteView()
{
	return {};
}

struct RHIViewDescription::BufferInfo::ViewInfo
{
	uint32 OffsetInBytes;
	uint32 StrideInBytes;
	uint32 ElementsNum;
	uint32 SizeInBytes;
	BufferType Type;
	bool IsNull;
};

struct RHIViewDescription::BufferReadViewInfo::ViewInfo : public RHIViewDescription::BufferInfo::ViewInfo
{
};

struct RHIViewDescription::BufferWriteViewInfo::ViewInfo : public RHIViewDescription::BufferInfo::ViewInfo
{
};

struct RHIViewDescription::TextureInfo::ViewInfo
{
	TextureDimensions Dimensions;
};

struct RHIViewDescription::TextureReadViewInfo::ViewInfo : public RHIViewDescription::TextureInfo::ViewInfo
{
};

struct RHIViewDescription::TextureWriteViewInfo::ViewInfo : public RHIViewDescription::TextureInfo::ViewInfo
{
};

class RHIView : public RHIResource
{
public:
	RHIView(RHIResourceType InResourceType, RHIViewDescription InViewDescription, RHIViewableResource* InResource)
		: RHIResource(InResourceType)
		  , ViewDescription(InViewDescription)
		  , Resource(InResource)
	{
	}

	RHIViewableResource* GetResource() const
	{
		return Resource;
	}

	RHIBuffer* GetBuffer() const
	{
		LE_ASSERT(ViewDescription.IsBufferView())
		return dynamic_cast<RHIBuffer*>(Resource.GetPointer());
	}

	RHITexture* GetTexture() const
	{
		LE_ASSERT(ViewDescription.IsTextureView())
		return dynamic_cast<RHITexture*>(Resource.GetPointer());
	}

	bool IsBufferView() const
	{
		return ViewDescription.IsBufferView();
	}

	bool IsTextureView() const
	{
		return ViewDescription.IsTextureView();
	}

	const RHIViewDescription& GetViewDescription() const
	{
		return ViewDescription;
	}

protected:
	RHIViewDescription ViewDescription;

private:
	RefCountingPtr<RHIViewableResource> Resource;
};

class RHIReadView : public RHIView
{
public:
	RHIReadView(RHIViewDescription InViewDescription, RHIViewableResource* InResource)
		: RHIView(RHIResourceType::ReadView, InViewDescription, InResource)
	{
		LE_ASSERT(ViewDescription.IsReadView())
	}
};

class RHIWriteView : public RHIView
{
public:
	RHIWriteView(RHIViewDescription InViewDescription, RHIViewableResource* InResource)
		: RHIView(RHIResourceType::WriteView, InViewDescription, InResource)
	{
		LE_ASSERT(ViewDescription.IsWriteView())
	}
};

class RHIRenderTargetView
{
public:
	RHIRenderTargetView() = default;

	RHIRenderTargetView(RHITexture* InTexture, RenderTargetLoadAction InLoadAction)
		: Texture(InTexture)
		  , LoadAction(InLoadAction)
		  , StoreAction(RenderTargetStoreAction::Store)
	{
	}

	RHITexture* Texture;
	RenderTargetLoadAction LoadAction = RenderTargetLoadAction::NoAction;
	RenderTargetStoreAction StoreAction = RenderTargetStoreAction::NoAction;
};

class RHIDepthRenderTargetView
{
public:
	RHIDepthRenderTargetView()
		: Texture(nullptr)
		  , DepthLoadAction(RenderTargetLoadAction::NoAction)
		  , DepthStoreAction(RenderTargetStoreAction::NoAction)
		  , StencilLoadAction(RenderTargetLoadAction::NoAction)
		  , StencilStoreAction(RenderTargetStoreAction::NoAction)
		  , DepthStencilAccess(ExclusiveDepthStencil::DepthNop_StencilNop)
	{
	}

	RHIDepthRenderTargetView(RHITexture* InTexture, RenderTargetLoadAction InLoadAction, RenderTargetStoreAction InStoreAction)
		: Texture(InTexture)
		  , DepthLoadAction(InLoadAction)
		  , DepthStoreAction(InStoreAction)
		  , StencilLoadAction(InLoadAction)
		  , StencilStoreAction(InStoreAction)
		  , DepthStencilAccess(ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{
	}

	RHIDepthRenderTargetView(RHITexture* InTexture, RenderTargetLoadAction InDepthLoadAction, RenderTargetStoreAction InDepthStoreAction,
	                         RenderTargetLoadAction InStencilLoadAction, RenderTargetStoreAction InStencilStoreAction,
	                         ExclusiveDepthStencil InDepthStencilAccess)
		: Texture(InTexture)
		  , DepthLoadAction(InDepthLoadAction)
		  , DepthStoreAction(InDepthStoreAction)
		  , StencilLoadAction(InStencilLoadAction)
		  , StencilStoreAction(InStencilStoreAction)
		  , DepthStencilAccess(InDepthStencilAccess)
	{
	}

	bool operator==(const RHIDepthRenderTargetView& Other) const
	{
		return Texture == Other.Texture &&
			DepthLoadAction == Other.DepthLoadAction &&
			DepthStoreAction == Other.DepthStoreAction &&
			StencilLoadAction == Other.StencilLoadAction &&
			StencilStoreAction == Other.StencilStoreAction &&
			DepthStencilAccess == Other.DepthStencilAccess;
	}

	RenderTargetStoreAction GetStencilStoreAction() const { return StencilStoreAction; }
	ExclusiveDepthStencil GetDepthStencilAccess() const { return DepthStencilAccess; }

public:
	RHITexture* Texture;

	RenderTargetLoadAction DepthLoadAction;
	RenderTargetStoreAction DepthStoreAction;
	RenderTargetLoadAction StencilLoadAction;

private:
	RenderTargetStoreAction StencilStoreAction;
	ExclusiveDepthStencil DepthStencilAccess;
};

struct BoundShadersState
{
	BoundShadersState() = default;

	RefCountingPtr<RHIVertexShader> GetVertexShader() const { return VertexShaderRHI; }
	RefCountingPtr<RHIPixelShader> GetPixelShader() const { return PixelShaderRHI; }

	RefCountingPtr<RHIVertexShader> VertexShaderRHI = nullptr;
	RefCountingPtr<RHIPixelShader> PixelShaderRHI = nullptr;
};

class PipelineStateInitializer
{
public:
	PipelineStateInitializer()
		: DepthStencilState(nullptr)
		  , Primitive(PrimitiveType::TriangleList)
		  , DepthStencilPixelFormat(PixelFormat::Invalid)
		  , DepthTargetLoadAction(RenderTargetLoadAction::NoAction)
		  , DepthTargetStoreAction(RenderTargetStoreAction::NoAction)
		  , StencilTargetLoadAction(RenderTargetLoadAction::NoAction)
		  , StencilTargetStoreAction(RenderTargetStoreAction::NoAction)
		  , DepthStencilAccess(ExclusiveDepthStencil::DepthNop_StencilNop)
	{
	}

	PipelineStateInitializer(BoundShadersState InShaderState,
	                         RHIDepthStencilState* InDepthStencilState,
	                         PrimitiveType InPrimitive,
	                         PixelFormat InDepthStencilPixelFormat,
	                         RenderTargetLoadAction InDepthTargetLoadAction,
	                         RenderTargetStoreAction InDepthTargetStoreAction,
	                         RenderTargetLoadAction InStencilTargetLoadAction,
	                         RenderTargetStoreAction InStencilTargetStoreAction,
	                         ExclusiveDepthStencil InDepthStencilAccess)
		: ShaderState(std::move(InShaderState))
		  , DepthStencilState(InDepthStencilState)
		  , Primitive(InPrimitive)
		  , DepthStencilPixelFormat(InDepthStencilPixelFormat)
		  , DepthTargetLoadAction(InDepthTargetLoadAction)
		  , DepthTargetStoreAction(InDepthTargetStoreAction)
		  , StencilTargetLoadAction(InStencilTargetLoadAction)
		  , StencilTargetStoreAction(InStencilTargetStoreAction)
		  , DepthStencilAccess(InDepthStencilAccess)
	{
	}

	BoundShadersState ShaderState;
	RHIDepthStencilState* DepthStencilState;

	PrimitiveType Primitive;
	PixelFormat DepthStencilPixelFormat;
	RenderTargetLoadAction DepthTargetLoadAction;
	RenderTargetStoreAction DepthTargetStoreAction;
	RenderTargetLoadAction StencilTargetLoadAction;
	RenderTargetStoreAction StencilTargetStoreAction;
	ExclusiveDepthStencil DepthStencilAccess;
};

class RHIPipelineStateObject : public RHIResource
{
public:
	RHIPipelineStateObject() : RHIResource(RHIResourceType::PipelineStateObject)
	{
	}
};

class RHINonNativePipelineStateObject : public RHIPipelineStateObject
{
public:
	RHINonNativePipelineStateObject(PipelineStateInitializer InInitializer)
		: Initializer(std::move(InInitializer))
	{
	}

	PipelineStateInitializer Initializer;
};

class RHIBoundShaderState : public RHIResource
{
public:
	RHIBoundShaderState()
		: RHIResource(RHIResourceType::BoundShaderState)
	{}
};


}
