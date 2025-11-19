#pragma once
#include <memory>
#include <vector>

#include "CoreDefinitions.h"
#include "ServiceBase.h"
#include "Math/Math.h"

namespace LE
{
class ServiceRegistry
{
public:
	ServiceRegistry() = default;

	void ShutDown();

	template <ServiceType Service>
	void RegisterService(std::unique_ptr<ServiceBase> RegisterService)
	{
		ServiceTypeId id = ServiceTypeIdGetter<Service>::Value;
		if (RegisteredServices.contains(id))
		{
			LE_ERROR("Trying to double register service {}", ServiceTypeIdGetter<Service>::TypeName);
		}
		
		RegisterService->Initialize();
		RegisteredServices.emplace(id, std::move(RegisterService));
	}

	template <ServiceType Service>
	Service& GetService()
	{
		ServiceTypeId id = ServiceTypeIdGetter<Service>::Value;
		if (!RegisteredServices.contains(id))
		{
			LE_ERROR("Trying to get unregistered service {}", ServiceTypeIdGetter<Service>::TypeName);
		}
		
		return static_cast<Service&>(*RegisteredServices.at(id));
	}

	template <ServiceType Service>
	const Service& GetService() const
	{
		ServiceTypeId id = ServiceTypeIdGetter<Service>::Value;
		if (!RegisteredServices.contains(id))
		{
			LE_ERROR("Trying to get unregistered service {}", ServiceTypeIdGetter<Service>::TypeName);
		}
		
		return static_cast<const Service&>(*RegisteredServices.at(id));
	}

private:
	std::unordered_map<ServiceTypeId, std::unique_ptr<ServiceBase>> RegisteredServices;
};

ServiceRegistry& GetServiceRegistry();
void RegisterServiceRegistry(ServiceRegistry* ServiceReg);
}
