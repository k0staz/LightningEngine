#pragma once
#include "LEWindow.h"
#include "Service/ServiceBase.h"
#include "Templates/RefCounters.h"

namespace LE
{
class SystemManager : public ServiceBase
{
public:
	void Initialize() override;
	void Shutdown() override;
	
	RefCountingPtr<LEWindow> CreateLEWindow(const std::string& WindowName, int32_t Width, int32_t Height);
	void DestroyWindow(RefCountingPtr<LEWindow> Window);
	bool PoolEvents();

};

REGISTER_SERVICE_TYPE(SystemManager, "SystemManager")
}
