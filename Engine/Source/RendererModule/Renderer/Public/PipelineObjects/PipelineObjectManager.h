#pragma once

#include "RHIResources.h"
#include "RenderContributors/RenderContributerCore.h"
#include "Service/ServiceBase.h"
#include "ShaderPass/ShaderPass.h"
#include "Templates/RefCounters.h"

namespace LE::Renderer
{
class PipelineObjectManager : public ServiceBase
{
public:
	struct PipelinePermutationKey
	{
		ShaderPassTypeId ShaderTypeId;
		RenderContributorTypeId GlobalContributorTypeId;
		RenderContributorTypeId MeshContributorTypeId;
		RenderContributorTypeId MaterialContributorTypeId;
		
		bool operator==(const PipelinePermutationKey&) const = default;
	};
	
	void Initialize() override;
	void Shutdown() override;
	
	RefCountingPtr<RHI::RHIPipelineLayout> GetPipelineLayout(const RHI::RHIPipelineLayoutDesc& Desc);
	RefCountingPtr<RHI::RHIPipelineObject> GetPipelineObject(const PipelinePermutationKey& Key);

private:
	struct PipelinePermutationKeyHash {
		size_t operator()(const PipelinePermutationKey& key) const noexcept {
			size_t seed = std::hash<ShaderPassTypeId>{}(key.ShaderTypeId);
			auto hash_combine = [](size_t& s, size_t v) {
				s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
			};
			hash_combine(seed, std::hash<RenderContributorTypeId>{}(key.GlobalContributorTypeId));
			hash_combine(seed, std::hash<RenderContributorTypeId>{}(key.MeshContributorTypeId));
			return seed;
		}
	};
	
	struct PipelineDescriptionKeyHash {
		size_t operator()(const RHI::RHIPipelineLayoutDesc& key) const noexcept {
			size_t seed = std::hash<ShaderPassTypeId>{}(key.PushConstantSize);
			auto hash_combine = [](size_t& s, size_t v) {
				s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
			};
			hash_combine(seed, std::hash<uint32>{}(static_cast<uint32>(key.ShaderStages)));
			return seed;
		}
	};
	
	std::unordered_map<RHI::RHIPipelineLayoutDesc, RefCountingPtr<RHI::RHIPipelineLayout>, PipelineDescriptionKeyHash> PipelineLayoutCacheMap;
	std::unordered_map<PipelinePermutationKey, RefCountingPtr<RHI::RHIPipelineObject>, PipelinePermutationKeyHash> PipelineObjectCacheMap;
};
}

namespace LE
{
REGISTER_SERVICE_TYPE(Renderer::PipelineObjectManager, "PipelineObjectManager")
}
