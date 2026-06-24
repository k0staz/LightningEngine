module MeshInterface;

__include "StaticMesh/StaticMesh.slang";

public interface IMeshFetch
{
    float4 GetPosition(uint vertexIdx, uint instanceIdx);
    float2 GetUV(uint vertexIdx, uint instanceIdx);
    float3 GetNormal(uint vertexIdx, uint instanceIdx);
}