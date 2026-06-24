#pragma once
#include "RHIResources.h"
#include "Service/ServiceBase.h"
#include "Templates/RefCounters.h"

namespace LE::Renderer
{
class RenderDynamicDataManager : public ServiceBase
{
public:
	void Initialize() override;
	void Shutdown() override;
	void OnBeginFrame() const;
	
	RefCountingPtr<RHI::RHILinearBuffer> GetCurrentFrameRingBuffer() const;
	
private:
	RefCountingPtr<RHI::RHILinearBuffer> FrameRingBuffer[DEFAULT_FRAMES_IN_FLIGHT];

};
}

namespace LE
{
REGISTER_SERVICE_TYPE(Renderer::RenderDynamicDataManager, "RenderDynamicDataManager")
}
