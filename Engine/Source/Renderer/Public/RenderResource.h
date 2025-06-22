#pragma once

#include "RHIResources.h"

namespace LE::Renderer
{
class RenderCommandList;

class RenderResource : public RefCountableBase
{
public:
	~RenderResource() override;

	virtual void InitRHI(RenderCommandList& CommandList)
	{
	}

	virtual void ReleaseRHI()
	{
	}

protected:
	static RefCountingPtr<RHI::RHIBuffer> CreateRHIBuffer(ResourceArrayInterface* ResourceData, RHI::BufferUsageFlags BufferUsage);
};

class VertexBuffer : public RenderResource
{
public:
	RefCountingPtr<RHI::RHIBuffer> VertexBufferRHI;
};

class IndexBuffer : public RenderResource
{
public:
	RefCountingPtr<RHI::RHIBuffer> IndexBufferRHI;
};

class TextureResource : public RenderResource
{
public:
	RefCountingPtr<RHI::RHITexture> TextureRHI;
};

template <class ResourceType>
class GlobalResource : public ResourceType
{
public:
	GlobalResource()
	{
		InitializeGlobalResource();
	}

	template <typename... Args>
	GlobalResource(Args... InArgs)
		: ResourceType(InArgs...)
	{
		InitializeGlobalResource();
	}

	virtual ~GlobalResource()
	{
		ReleaseGlobalResource();
	}

private:
	void InitializeGlobalResource()
	{
		// TODO: This needs to be executed only when in rendering thread
		static_cast<ResourceType*>(this)->InitRHI(RenderCommandList::Get());
	}

	void ReleaseGlobalResource()
	{
		static_cast<ResourceType*>(this)->ReleaseRHI();
	}
};

template <uint32 Size>
class BoundShaderStateCash : public RenderResource
{
public:
	BoundShaderStateCash() = default;

	void Add(RHI::RHIBoundShaderState* BoundShaderState)
	{
		BoundShaderStates[NextIndex] = BoundShaderState;
		NextIndex = (NextIndex + 1) & Size;
	}

	RHI::RHIBoundShaderState* GetLast()
	{
		uint32 lastIndex = NextIndex == 0 ? Size - 1 : NextIndex - 1;
		return BoundShaderStates[lastIndex];
	}

	void ReleaseRHI() override
	{
		for (auto& it : BoundShaderStates)
		{
			it->Release();
		}
	}

private:
	RefCountingPtr<RHI::RHIBoundShaderState> BoundShaderStates[Size];
	uint32 NextIndex = 0;
};
}
