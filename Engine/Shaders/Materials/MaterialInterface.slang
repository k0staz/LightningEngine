module MaterialInterface;

__include "BaseMaterial/BaseMaterial.slang";
__include "ColorMaterial/ColorMaterial.slang";

public interface IMaterialInterface
{
    float4 GetBaseColor(float2 uv);
}
