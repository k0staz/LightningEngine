module MaterialInterface;

__include "BaseMaterial/BaseMaterial.slang";

public interface IMaterialInterface
{
    float4 GetBaseColor(float2 uv);
}
