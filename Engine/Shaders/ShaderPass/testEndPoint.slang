module testEndPoint;

import Globals.Global;
import Mesh.MeshInterface;

struct PushConstants<G :IGlobal, M : IMeshFetch>
{
    G GlobalFrameData;
    M MeshData;
}

struct VertexOutput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
    uint32_t InstanceIndex;
}

[shader("vertex")]
VertexOutput vertexMain<G : IGlobal, M : IMeshFetch>(uint vertexIdx: SV_VulkanVertexID, uint instanceIdx: SV_VulkanInstanceID, uniform PushConstants<G, M> pc)
{
    VertexOutput output;
    output.Position = mul(pc.MeshData.GetPosition(vertexIdx, instanceIdx), pc.GlobalFrameData.GetWorldToView());
    output.Position = mul(output.Position, pc.GlobalFrameData.GetViewToClip());

    output.Normal = mul((float3x3)(pc.GlobalFrameData.GetWorldToView()), pc.MeshData.GetNormal(vertexIdx, instanceIdx));
    output.UV = pc.MeshData.GetUV(vertexIdx, instanceIdx);

    output.InstanceIndex = instanceIdx;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input)
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f); 
}