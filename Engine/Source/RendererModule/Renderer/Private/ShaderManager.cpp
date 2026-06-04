#include "ShaderManager.h"

#include <utility>

#include "DynamicRHI.h"
#include "RenderCommandList.h"
#include "ShaderCompiler.h"
#include "MeshConverters/MeshConverter.h"


namespace LE::Renderer
{
void ShaderMapping::InitRHI(RenderCommandList& CommandList)
{
	RenderResource::InitRHI(CommandList);
}

RefCountingPtr<ShaderMapping> ShaderMap::GetShaderMapping(const ShaderVariantKey& VariantKey) const
{
	auto it = VariantMap.find(VariantKey);
	if (it == VariantMap.end())
	{
		return nullptr;
	}

	return it->second;
}

void ShaderMap::AddShaderMapping(const ShaderVariantKey& VariantKey, RefCountingPtr<ShaderMapping> NewMapping)
{
	VariantMap[VariantKey] = std::move(NewMapping);
}

ShaderManager* ShaderManager::GShaderManager = nullptr;

ShaderManager* ShaderManager::Get()
{
	if (!GShaderManager)
	{
		GShaderManager = new ShaderManager;
	}

	return GShaderManager;
}

RefCountingPtr<ShaderMapping> ShaderManager::GetShaderMapping(const ShaderLookUpKey& LookUpKey)
{
	ShaderManager* shaderManager = Get();

	ShaderMap& shaderMap = shaderManager->ShaderMaps[LookUpKey.ShaderType];

	RefCountingPtr<ShaderMapping> mapping = shaderMap.GetShaderMapping(LookUpKey.VariantKey);
	if (!mapping)
	{
		mapping = shaderManager->CompileShaderVariant(LookUpKey);
	}

	return mapping;
}

RefCountingPtr<ShaderMapping> ShaderManager::CompileShaderVariant(const ShaderLookUpKey& LookUpKey)
{
	ShaderCompilerResult compilerResult;
	if (!CompileShader(LookUpKey.ShaderType, LookUpKey.MCType, compilerResult))
	{
		LE_ERROR("Failed to compile shader {} for MC type {}", LookUpKey.ShaderType->GetName(), LookUpKey.MCType->GetName());
		return nullptr;
	}

	CompiledShaderInitializer initializer(LookUpKey.ShaderType, compilerResult, LookUpKey.MCType);
	Shader* logicalShader = new Shader(initializer);

	RefCountingPtr<RHI::RHIShader> rhiShader = nullptr;
	if (LookUpKey.ShaderType->GetShaderType() == RHI::ShaderType::Vertex)
	{
		rhiShader = RHI::RHICreateVertexShader(compilerResult.Code);
	}
	else
	{
		rhiShader = RHI::RHICreatePixelShader(compilerResult.Code);
	}

	RefCountingPtr<ShaderMapping> newMapping = new ShaderMapping(logicalShader, rhiShader);

	ShaderMap& shaderMap = ShaderMaps[LookUpKey.ShaderType];
	shaderMap.AddShaderMapping(LookUpKey.VariantKey, newMapping);

	RenderCommandList::Get().EnqueueLambdaCommand([newMapping](RenderCommandList& CmdList)
	{
		newMapping->InitRHI(CmdList);
	});

	return newMapping;
}
}
