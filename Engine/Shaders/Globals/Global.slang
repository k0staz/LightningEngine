module Global;

__include "GlobalStandart/GlobalStandard.slang";

public interface IGlobal
{
    public float4x4 GetWorldToView();
    public float4x4 GetViewToClip();
}