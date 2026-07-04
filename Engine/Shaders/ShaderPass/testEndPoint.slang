module testEndPoint;

import Globals.Global;
import Mesh.MeshInterface;
import Materials.MaterialInterface;

struct PushConstants<G :IGlobal, M : IMeshFetch, Material : IMaterialInterface>
{
    G GlobalFrameData;
    M MeshData;
    Material MaterialData;
}

struct VertexOutput<Material : IMaterialInterface>
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
    uint32_t InstanceIndex;
    Material Material;
}

[shader("vertex")]
VertexOutput<Material> vertexMain<G : IGlobal, M : IMeshFetch, Material : IMaterialInterface>(uint vertexIdx: SV_VulkanVertexID, uint instanceIdx: SV_VulkanInstanceID, uniform PushConstants<G, M, Material> pc)
{
    VertexOutput<Material> output;
    output.Material = pc.MaterialData;
    output.Position = mul(pc.MeshData.GetPosition(vertexIdx, instanceIdx), pc.GlobalFrameData.GetWorldToView());
    output.Position = mul(output.Position, pc.GlobalFrameData.GetViewToClip());

    output.Normal = mul((float3x3)(pc.GlobalFrameData.GetWorldToView()), pc.MeshData.GetNormal(vertexIdx, instanceIdx));
    output.UV = pc.MeshData.GetUV(vertexIdx, instanceIdx);

    output.InstanceIndex = instanceIdx;

    return output;
}

[shader("fragment")]
float4 fragmentMain<Material : IMaterialInterface>(VertexOutput<Material> input)
{
    return input.Material.GetBaseColor(input.UV); 
}