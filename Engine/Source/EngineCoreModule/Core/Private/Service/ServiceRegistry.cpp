#include "Service/ServiceRegistry.h"

namespace LE
{
static ServiceRegistry* gServiceRegistry = nullptr;

void ServiceRegistry::ShutDown()
{
	for (auto& it : RegisteredServices)
	{
		it.second->Shutdown();
	}

	RegisteredServices.clear();
}

ServiceRegistry& GetServiceRegistry()
{
	if (!gServiceRegistry)
	{
		LE_ASSERT_DESC(false, "Service Registry is not initialized")
	}

	return *gServiceRegistry;
}

void RegisterServiceRegistry(ServiceRegistry* ServiceReg)
{
	if (gServiceRegistry)
	{
		LE_ERROR("Trying to double initialize service registry");
		return;
	}

	gServiceRegistry = ServiceReg;
}
}
