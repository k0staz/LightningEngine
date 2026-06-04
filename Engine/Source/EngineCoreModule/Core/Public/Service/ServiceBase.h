#pragma once
#include "Templates/NonCopyable.h"
#include "CoreDefinitions.h"

namespace LE
{
class ServiceBase : public NonCopyable
{
public:
	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;

	virtual ~ServiceBase() = default;
};

template <typename Service>
concept ServiceType = std::is_base_of_v<ServiceBase, Service>;

template <ServiceType Service>
struct ServiceTypeRegistration;

using ServiceTypeId = uint32;

template <ServiceType Service>
struct ServiceTypeIdGetter
{
	static constexpr std::string_view TypeName = ServiceTypeRegistration<Service>::Value;
	static constexpr ServiceTypeId Value = FNV1AHash(TypeName);
};

#define REGISTER_SERVICE_TYPE(ServiceType, ServiceName) \
	template<> \
	struct ServiceTypeRegistration<ServiceType> \
	{ \
		static constexpr std::string_view Value = ServiceName; \
	};
}
