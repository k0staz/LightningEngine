implementing Global;

public struct GlobalFrameData
{
    public float4x4 WorldToView;
    public float4x4 ViewToClip;
}

public struct GlobalStandard : IGlobal
{
    public GlobalFrameData* FrameDataPtr;

    public float4x4 GetWorldToView()
    {
        return FrameDataPtr->WorldToView;
    }

    public float4x4 GetViewToClip()
    {
        return FrameDataPtr->ViewToClip;
    }
}