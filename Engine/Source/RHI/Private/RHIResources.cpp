#include "RHIResources.h"

#include "RHIConstantBufferInitializer.h"

namespace LE::RHI
{
RHIConstantBufferResource::RHIConstantBufferResource(const RHIConstantBufferResourceInitializer& Initializer)
	: Name(Initializer.Name)
	  , ResourceOffset(Initializer.ResourceOffset)
	  , ResourceType(Initializer.ResourceType)
{
}

RHIConstantBufferLayout::RHIConstantBufferLayout(const RHIConstantBufferInitializer& Initializer)
	: RHIResource(RHIResourceType::ConstantBufferLayout)
	  , Name(Initializer.Name)
	  , Resources(Initializer.Resources.begin(), Initializer.Resources.end())
	  , ConstantBufferSize(Initializer.ConstantBufferSize)
{
}

RHIViewDescription::BufferInfo::ViewInfo RHIViewDescription::BufferInfo::GetViewInfo(const RHIBuffer* Buffer) const
{
	LE_ASSERT(Buffer)
	LE_ASSERT(TypeBuffer != BufferType::Invalid)

	const RHIBufferDesc& desc = Buffer->GetDescription();
	if (desc.IsNull())
	{
		ViewInfo info{};
		info.IsNull = true;
		return info;
	}

	ViewInfo info{};
	info.Type = TypeBuffer;

	switch (info.Type)
	{
	case BufferType::Invalid:
		LE_ASSERT(false)
		break;
	case BufferType::Typed:
		info.StrideInBytes = GPixelFormats[static_cast<uint8>(Format)].BlockBytes;
		break;
	case BufferType::Structured:
		info.StrideInBytes = Stride == 0 ? desc.Stride : Stride;
		break;
	case BufferType::Raw:
		info.StrideInBytes = sizeof(uint32);
		break;
	}

	info.OffsetInBytes = OffsetInBytes;
	info.ElementsNum = ElementsNum == 0 ? (desc.Size - OffsetInBytes) / info.StrideInBytes : ElementsNum;
	info.SizeInBytes = info.ElementsNum * info.StrideInBytes;

	return info;
}

RHIViewDescription::BufferReadViewInfo::ViewInfo RHIViewDescription::BufferReadViewInfo::GetViewInfo(const RHIBuffer* Buffer) const
{
	return ViewInfo(RHIViewDescription::BufferInfo::GetViewInfo(Buffer));
}

RHIViewDescription::BufferWriteViewInfo::ViewInfo RHIViewDescription::BufferWriteViewInfo::GetViewInfo(const RHIBuffer* Buffer) const
{
	return ViewInfo(RHIViewDescription::BufferInfo::GetViewInfo(Buffer));
}

RHIViewDescription::TextureInfo::ViewInfo RHIViewDescription::TextureInfo::GetViewInfo(const RHITexture* Texture) const
{
	LE_ASSERT(Texture)

	const RHITextureDesc& desc = Texture->GetDescription();
	LE_ASSERT(Dimensions == desc.Dimension)

	ViewInfo info{};
	info.Dimensions = desc.Dimension;

	return info;
}

RHIViewDescription::TextureReadViewInfo::ViewInfo RHIViewDescription::TextureReadViewInfo::GetViewInfo(const RHITexture* Texture) const
{
	return ViewInfo(RHIViewDescription::TextureInfo::GetViewInfo(Texture));
}

RHIViewDescription::TextureWriteViewInfo::ViewInfo RHIViewDescription::TextureWriteViewInfo::GetViewInfo(const RHITexture* Texture) const
{
	return ViewInfo(RHIViewDescription::TextureInfo::GetViewInfo(Texture));
}
}
