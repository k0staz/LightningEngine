#pragma once
#include "RHIResources.h"
#include "Templates/RefCounters.h"

namespace LE::Renderer
{
enum class RGState : uint8
{
    None,
    ColorAttachment,
    DepthAttachment,
    Present,
};

struct RGPhysicalState
{
    RHI::RHIImageLayout Layout;
    RHI::RHIPipelineStageFlags Stage;
    RHI::RHIAccessFlags Access;
};

constexpr RGPhysicalState ToPhysical(RGState State)
{
    using L = RHI::RHIImageLayout;
    using P = RHI::RHIPipelineStageFlags;
    using A = RHI::RHIAccessFlags;
    switch (State)
    {
    case RGState::ColorAttachment: return { L::Attachment, P::ColorTarget, A::ColorRead | A::ColorWrite };
    case RGState::DepthAttachment: return { L::Attachment, P::DepthTarget, A::DepthWrite };
    case RGState::Present:         return { L::Present,    P::ColorTarget,   A::None };
    case RGState::None:
    default:                       return { L::None,       P::ColorTarget,   A::None };
    }
}

struct RGTexture
{
    uint32 Index = ~0u;
    bool IsValid() const { return Index != ~0u; }
};

struct RGTextureEntry
{
    RefCountingPtr<RHI::RHIImage> Image;
    RefCountingPtr<RHI::RHIImageView> View;
    RGPhysicalState CurrentState;
    RGState FinalState = RGState::None;
    RHI::RHISubresourceRange Range;
    bool IsImported = false;
};
}
