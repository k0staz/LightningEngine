implementing MaterialInterface;

import TextureResources;

public struct ColorMaterialFrameData
{
    float4 Color;
}

public struct ColorMaterial : IMaterialInterface
{
    ColorMaterialFrameData* FrameData;

    float4 GetBaseColor(float2 uv)
    {
        return FrameData->Color;
    }
}