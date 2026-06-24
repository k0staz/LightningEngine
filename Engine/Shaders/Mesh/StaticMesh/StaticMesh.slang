implementing MeshInterface;

public struct StaticMeshResources
{
    uint* Indices_Fetch;
    float3* Positions_Fetch;
    float3* Normals_Fetch;
    float2* UV_Fetch;
};

public struct MeshInstanceData
{
    float4x4 LocalToWorld;
}

public struct StaticMeshFrameData
{
    public StaticMeshResources* ResourcesPtr;
    public MeshInstanceData* InstancesPtr;
}

public struct StaticMeshFetch : IMeshFetch
{
    StaticMeshFrameData* FrameData;

    uint GetVertex(uint vertexIdx)
    {
        return FrameData->ResourcesPtr->Indices_Fetch[vertexIdx];
    }

    public float4 GetPosition(uint vertexIdx, uint instanceIdx)
    {
        uint fetchedVertex = GetVertex(vertexIdx);
        float4 position = float4(FrameData->ResourcesPtr->Positions_Fetch[fetchedVertex], 1);
        position = mul(position, FrameData->InstancesPtr[instanceIdx].LocalToWorld);
        return position;
    }

    public float2 GetUV(uint vertexIdx, uint instanceIdx)
    {
        uint fetchedVertex = GetVertex(vertexIdx);
        return FrameData->ResourcesPtr->UV_Fetch[fetchedVertex];
    }

    public float3 GetNormal(uint vertexIdx, uint instanceIdx)
    {
        uint fetchedVertex = GetVertex(vertexIdx);
        float3 normal = FrameData->ResourcesPtr->Normals_Fetch[fetchedVertex];
        normal = mul(normal, float3x3(FrameData->InstancesPtr[instanceIdx].LocalToWorld));
        return normal;
    }
}
