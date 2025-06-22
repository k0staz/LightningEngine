cbuffer ViewShaderParametersConstantBuffer : register(b0)
{
    matrix WorldToView;
    matrix ViewToClip;
};

cbuffer ObjectShaderParameters : register(b1)
{
    matrix LocalToWorld;
}

Buffer<float4> VertexFetch_PositionBuffer : register(t0);
Buffer<float4> VertexFetch_TangentBuffer : register(t1);
Buffer<float4> VertexFetch_TexCoordBuffer : register(t2);

struct PixelInputType
{
    float4 position : SV_POSITION;
};

PixelInputType VSMain(uint vertexID : SV_VertexID)
{
    PixelInputType output;
    
    output.position = mul(VertexFetch_PositionBuffer.Load(vertexID), LocalToWorld);
    output.position = mul(output.position, WorldToView);
    output.position = mul(output.position, ViewToClip);

    return output;
}