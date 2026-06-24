#include "PipelineObjects/PipelineObjectManager.h"

#include "RHIDevice.h"
#include "ShaderVariationRegistry.h"
#include "Service/ServiceRegistry.h"
#include "tracy/Tracy.hpp"

namespace LE::Renderer
{

void PipelineObjectManager::Initialize() {}

void PipelineObjectManager::Shutdown()
{
	RHI::RHIDevice* device = RHI::RHIDevice::Get();
	for (auto& [key, pipelineLayout] : PipelineLayoutCacheMap)
	{
		device->DestroyPipelineLayout(pipelineLayout);
	}

	for (auto& [key, pipelineObject] : PipelineObjectCacheMap)
	{
		device->DestroyPipelineObject(pipelineObject);
	}

	PipelineLayoutCacheMap.clear();
	PipelineObjectCacheMap.clear();
}

RefCountingPtr<RHI::RHIPipelineLayout> PipelineObjectManager::GetPipelineLayout(const RHI::RHIPipelineLayoutDesc& Desc)
{
	ZoneScopedN("[PipelineObjectManager] Get Pipeline Layout");
	auto it = PipelineLayoutCacheMap.find(Desc);
	if (it != PipelineLayoutCacheMap.end())
	{
		return it->second;
	}
	
	RHI::RHIDevice* device = RHI::RHIDevice::Get();
	PipelineLayoutCacheMap[Desc] = device->CreatePipelineLayout(Desc);
	return PipelineLayoutCacheMap[Desc];
}

RefCountingPtr<RHI::RHIPipelineObject> PipelineObjectManager::GetPipelineObject(const PipelinePermutationKey& Key)
{
	ZoneScopedN("[PipelineObjectManager] Get Pipeline Object");
	auto it = PipelineObjectCacheMap.find(Key);
	if (it != PipelineObjectCacheMap.end())
	{
		return it->second;
	}

	ShaderVariationRegistry& shaderVariationReg = GetServiceRegistry().GetService<ShaderVariationRegistry>();
	ShaderPassVariation shaderPassDesc = shaderVariationReg.GetShaderPassVariation(Key.ShaderTypeId);
	if (shaderPassDesc.Path.empty())
	{
		LE_ASSERT_DESC(false, "Shader pass not found");
		return nullptr;
	}

	RHI::RHIPipelineObjectDesc pipelineObjectDesc = {};

	RHI::RHIPipelineLayoutDesc layoutDesc = {};
	layoutDesc.ShaderStages = shaderPassDesc.PushConstantStages;
	layoutDesc.PushConstantSize = shaderPassDesc.PushConstantSize;

	pipelineObjectDesc.PipelineLayout = GetPipelineLayout(layoutDesc);
	pipelineObjectDesc.Stages = shaderPassDesc.Stages;

	RHI::ShaderCompiler& shaderCompiler = GetServiceRegistry().GetService<RHI::ShaderCompiler>();
	RHI::ShaderCompiler::SpecializedProgramDesc shaderDesc = {};
	shaderDesc.ShaderPath = shaderPassDesc.Path;
	shaderDesc.ShaderStage = shaderPassDesc.Stages;

	if (Key.GlobalContributorTypeId != NullId{})
	{
		auto& globalVariation = shaderDesc.ModuleSpecializations.emplace_back();
		globalVariation = shaderVariationReg.GetRenderContributorData(Key.GlobalContributorTypeId);
	}

	if (Key.MeshContributorTypeId != NullId{})
	{
		auto& meshVariation = shaderDesc.ModuleSpecializations.emplace_back();
		meshVariation = shaderVariationReg.GetRenderContributorData(Key.MeshContributorTypeId);
	}
	RHI::ShaderCompiler::SpecializedProgram specializedProgram = shaderCompiler.CompileProgram(shaderDesc);
	pipelineObjectDesc.ShaderModule = specializedProgram.CompiledModule;

	RHI::RHIDevice* device = RHI::RHIDevice::Get();
	RefCountingPtr<RHI::RHIPipelineObject> pipelineObject = device->CreatePipelineObject(pipelineObjectDesc);
	PipelineObjectCacheMap[Key] = pipelineObject;

	return pipelineObject;
}
}
