#pragma once
#include <type_traits>

#include "CoreDefinitions.h"
#include "Math/Math.h"
#include "RHIDefinitions.h"

namespace LE::Renderer
{
struct ShaderPassBase
{
};

template <typename ShaderPass>
concept ShaderPassType = std::is_base_of_v<ShaderPassBase, ShaderPass>;

template<ShaderPassType ShaderPass>
struct ShaderPassRegistration;

using ShaderPassTypeId = uint32;

template<ShaderPassType ShaderPass>
struct ShaderPassGetter
{
	using PassConstants = typename ShaderPassRegistration<ShaderPass>::ThisPassConstants;
	static constexpr const char* Name = ShaderPassRegistration<ShaderPass>::Name;
	static constexpr const char* Path = ShaderPassRegistration<ShaderPass>::Path;
	static constexpr RHI::RHIShaderStage Stages = ShaderPassRegistration<ShaderPass>::Stages;
	static constexpr RHI::RHIShaderStage PushConstantStages = ShaderPassRegistration<ShaderPass>::PushConstantStages;
	static constexpr ShaderPassTypeId Value = FNV1AHash(Name);
};

struct ShaderPassVariation
{
	std::string_view Path;
	RHI::RHIShaderStage Stages;
	RHI::RHIShaderStage PushConstantStages;
	uint64 PushConstantSize;
};

#define REGISTER_SHADER_PASS_TYPE(ShaderPassType, ShaderPassName, ShaderStages, ShaderPushConstantStages, ShaderPassPath) \
	template<> \
	struct ShaderPassRegistration<ShaderPassType> \
	{ \
		using ThisPassConstants = ShaderPassType::PushConstants; \
		static constexpr const char* Name = ShaderPassName; \
		static constexpr const char* Path = ShaderPassPath; \
		static constexpr RHI::RHIShaderStage Stages = ShaderStages; \
		static constexpr RHI::RHIShaderStage PushConstantStages = ShaderPushConstantStages; \
	};

struct TestShaderPass : ShaderPassBase
{
	struct alignas(16) PushConstants
	{
		uint64 GlobalFrameDataGpuAddress = 0;
		uint64 MeshFrameDataGpuAddress = 0;
	};
};

REGISTER_SHADER_PASS_TYPE(LE::Renderer::TestShaderPass, "TestShaderPass", RHI::RHIShaderStage::AllGraphics, RHI::RHIShaderStage::Vertex, "ShaderPass/testEndPoint")
}
