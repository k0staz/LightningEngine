implementing MaterialInterface;

import TextureResources;

public struct BaseMaterialFrameData
{
    uint64_t BaseColorTextureIndex;
    uint BaseColorSamplerIndex;
}

public struct BaseMaterial : IMaterialInterface
{
    BaseMaterialFrameData* FrameData;

    float4 GetBaseColor(float2 uv)
    {
        return GlobalTextures[FrameData->BaseColorTextureIndex].Sample(GlobalSamplers[FrameData->BaseColorSamplerIndex], uv);
    }
}