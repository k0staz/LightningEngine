#pragma once


namespace LE::RHI
{
class RHIResource;
}

namespace LE::Renderer
{

class RGResource
{
public:
	RGResource(const RGResource&) = delete;
	virtual ~RGResource() = default;

	RHI::RHIResource* GetRHI() const
	{
		return ResourceRHI;
	}

	bool HasRHIResource() const
	{
		return ResourceRHI != nullptr;
	}

private:
	RHI::RHIResource* ResourceRHI = nullptr;
};

class RGViewableResource : public RGResource
{
	
};

class RGBuffer final : public RGViewableResource
{
	
};

class RGTexture final : public RGViewableResource
{

};

class RGView : public RGResource
{
	
};

class RGReadViewResource : public RGView
{
	
};

class RGTextureReadView final : public RGReadViewResource
{
	
};

class RGBufferReadView final : public RGReadViewResource
{

};

class RGWriteViewResource : public RGView
{
	
};

class RGTextureWriteView final : public RGWriteViewResource
{

};

class RGBufferWriteView final : public RGWriteViewResource
{

};

}
