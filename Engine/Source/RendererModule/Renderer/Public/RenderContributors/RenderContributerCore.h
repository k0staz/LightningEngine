#pragma once
#include "CoreDefinitions.h"
#include "Math/Math.h"
#include "Misc/Traits.h"

namespace LE::Renderer
{
using RenderContributorId = uint32;
using RenderContributorTypeId = uint32;

enum class RenderContributorMetaType : uint8
{
	GlobalData,
	
	MeshData,
	MaterialData,
};

template<typename Type>
struct RenderContributorTraits : IdTraitsInterpreter<IdTraits<Type>>
{
	static constexpr uint64 PageSize = 4096;
};

inline constexpr NullId RenderContributorIdNull{};
inline constexpr NullId RenderContributorTypeIdNull{};

template <typename Type>
struct RenderContributorTypeRegistration;

template<typename Type>
struct RenderContributorTypeIdGetter
{
	static constexpr std::string_view TypeName = RenderContributorTypeRegistration<Type>::Value;
	static constexpr std::string_view InterfaceValue = RenderContributorTypeRegistration<Type>::InterfaceValue;
	static constexpr std::string_view Path = RenderContributorTypeRegistration<Type>::Path;
	static constexpr RenderContributorTypeId Value = FNV1AHash(TypeName); 
};

template<typename Type>
concept DerivedFromRenderContributor = std::is_base_of_v<class RenderContributor, Type>;

#define REGISTER_RENDER_CONTRIBUTOR_TYPE(Type, TypeName, InterfaceName, ShaderUnitPath) \
	template<> \
	struct RenderContributorTypeRegistration<Type> \
	{\
	static constexpr std::string_view Value = TypeName; \
	static constexpr std::string_view InterfaceValue = InterfaceName; \
	static constexpr std::string_view Path = ShaderUnitPath; \
	};
}
