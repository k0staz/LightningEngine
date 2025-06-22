cbuffer RootConstantBuffer
{
    float4 Color;
}

struct PixelInputType
{
    float4 position : SV_POSITION;
};

float4 PSMain(PixelInputType input): SV_Target
{
    float4 textureColor;
    
    textureColor = Color;
    
    return textureColor;
}