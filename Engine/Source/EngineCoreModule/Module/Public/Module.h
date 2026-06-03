#pragma once
#include <type_traits>

#include "CoreDefinitions.h"
#include "Math/Math.h"
#include "Templates/NonCopyable.h"

namespace LE
{
/**
 * @brief Base virtual class for modules
 */
class ModuleBase : public NonCopyable
{
public:
	virtual ~ModuleBase() = default;
	/**
	 * @brief Registers services which this module uses
	 */
	virtual void RegisterServices() = 0;
	
	virtual void ShutdownServices() = 0;

	/**
	 * @brief Registers reflection introduced by this module
	 */
	virtual void RegisterReflection() = 0;
};

template <typename Module>
concept ModuleType = std::is_base_of_v<ModuleBase, Module>;

template <ModuleType Module>
struct ModuleTypeRegistration;

using ModuleTypeId = uint32;

template <ModuleType Module>
struct ModuleTypeIdGetter
{
	static constexpr std::string_view TypeName = ModuleTypeRegistration<Module>::Value;
	static constexpr ModuleTypeId Value = FNV1AHash(TypeName);
};

#define REGISTER_MODULE(ModuleType, ModuleName) \
template<> \
struct ModuleTypeRegistration<ModuleType> \
{ \
static constexpr std::string_view Value = ModuleName; \
};

}
