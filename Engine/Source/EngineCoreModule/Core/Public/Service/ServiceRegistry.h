#pragma once
#include <memory>
#include <vector>

#include "CoreDefinitions.h"
#include "ServiceBase.h"
#include "Math/Math.h"

namespace LE
{
/**
 * @brief Service Registry for the engine.
 * Each module is responsible for registering and unregistering service which it is utilizing.
 */
class ServiceRegistry
{
public:
	ServiceRegistry() = default;

	void ShutDown();

	/**
	 * @brief Registers services. Each service can be registered only once.
	 * @tparam Service Service Type which should be registered
	 */
	template <ServiceType Service>
	void RegisterService()
	{
		ServiceTypeId id = ServiceTypeIdGetter<Service>::Value;
		if (RegisteredServices.contains(id))
		{
			LE_ERROR("Trying to double register service {}", ServiceTypeIdGetter<Service>::TypeName);
			return;
		}
		
		std::unique_ptr<Service> newService = std::make_unique<Service>();
		newService->Initialize();
		RegisteredServices.emplace(id, std::move(newService));
	}

	/**
	 * @brief Unregisters service
	 * @tparam Service Service type which should be unregistered
	 */
	template<ServiceType Service>
	void UnregisterService()
	{
		ServiceTypeId id = ServiceTypeIdGetter<Service>::Value;
		if (!RegisteredServices.contains(id))
		{
			LE_ERROR("Trying to unregister service {} which is not registered", ServiceTypeIdGetter<Service>::TypeName);
			return;
		}
		
		RegisteredServices.at(id)->Shutdown();
		RegisteredServices.erase(id);
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
