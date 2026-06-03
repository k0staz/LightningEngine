#include "Materials/Material.h"

#include "DynamicRHI.h"
#include "ShaderParameterMetadata.h"
#include "MeshPassCommandBuilders/MeshPassCommandBuilder.h"
#include "SceneRendering/SceneView.h"

namespace LE::Renderer
{
MaterialShaderMetaType::MaterialShaderMetaType(const char* InName, const char* InSourceFileName, const char* InMainFunctionName,
                                               const char* InMaterialName, RHI::ShaderType InShaderType, RenderPassType InRenderPassType,
                                               CreateCompiledType InCreateCompiledRef,
                                               const ShaderParametersMetadata* InParameterMetadata)
	: ShaderMetaType(InName, InSourceFileName, InMainFunctionName, InShaderType, InCreateCompiledRef, InParameterMetadata)
	  , MaterialName(InMaterialName)
	  , RenderPass(InRenderPassType)
{
	Material::MaterialRegistry& registry = Material::GetMaterialRegistry();
	Material*& material = registry[MaterialName];
	if (!material)
	{
		material = new Material(MaterialName);
	}

	Material::ShaderSet& shaderSet = material->RenderPassShaders[RenderPass];
	LE_ASSERT(!shaderSet[static_cast<uint8>(ShaderType)])

	shaderSet[static_cast<uint8>(ShaderType)] = this;
}

Map<String, Material*>& Material::GetMaterialRegistry()
{
	static MaterialRegistry materialMap;
	return materialMap;
}

Material* Material::GetMaterialByName(const String& Name)
{
	const Map<String, Material*>& registry = GetMaterialRegistry();
	const auto& it = registry.find(Name);
	if (it == registry.end())
	{
		return nullptr;
	}

	return it->second;
}

const Material::ShaderSet* Material::GetShaderSetForRenderPass(const RenderPassType RenderPass) const
{
	const auto& iterator = RenderPassShaders.find(RenderPass);
	if (iterator != RenderPassShaders.end())
	{
		return &iterator->second;
	}

	return nullptr;
}

ResolvedMaterialShaderMapping Material::GetShaderMappings(const RenderPassType RenderPass, MeshConverterType* MCType) const
{
	const ShaderSet* shaderSet = GetShaderSetForRenderPass(RenderPass);
	if (!shaderSet || !MCType)
	{
		return {};
	}

	ShaderLookUpKey vertexLookUpKey;
	vertexLookUpKey.ShaderType = (*shaderSet)[static_cast<uint8>(RHI::ShaderType::Vertex)];
	vertexLookUpKey.MCType = MCType;
	vertexLookUpKey.VariantKey.MaterialVariant = 0;

	ShaderLookUpKey pixelLookUpKey;
	pixelLookUpKey.ShaderType = (*shaderSet)[static_cast<uint8>(RHI::ShaderType::Pixel)];
	pixelLookUpKey.MCType = MCType;
	pixelLookUpKey.VariantKey.MaterialVariant = 0;

	ResolvedMaterialShaderMapping result;
	result.VertexShaderMapping = ShaderManager::GetShaderMapping(vertexLookUpKey);
	result.PixelShaderMapping = ShaderManager::GetShaderMapping(pixelLookUpKey);

	return result;
}

MaterialInstance::MaterialInstance(const Renderer::Material* InMaterial)
	: Material(InMaterial)
{
	LE_ASSERT(Material)

	const Map<RenderPassType, Material::ShaderSet>& shaders = Material->GetShaders();
	for (auto& shaderSetIt : shaders)
	{
		for (auto& shader : shaderSetIt.second)
		{
			AllocateShaderParameters(shader);
		}
	}
}

MaterialInstance::~MaterialInstance()
{
	for (auto& it : Data)
	{
		delete it.second;
	}
}

void MaterialInstance::SetParameter(const MaterialShaderMetaType* ShaderMetaType, const String& ParameterName, const uint8* ParameterValue)
{
	if (!ShaderMetaType)
	{
		LE_ASSERT(false)
		return;
	}

	if (!Data.contains(ShaderMetaType))
	{
		LE_ASSERT_DESC(false, "Trying to set parameter for unallocated shader")
		return;
	}

	const ShaderParametersMetadata* metadata = ShaderMetaType->GetShaderParameterMetadata();
	if (!metadata)
	{
		LE_ASSERT(false)
		return;
	}

	const ShaderParametersMetadata::Parameter* parameter = metadata->GetParameter(ParameterName);
	if (!parameter)
	{
		LE_ASSERT_DESC(false, "Failed to find a parameter")
		return;
	}

	uint8* shaderData = Data[ShaderMetaType];
	memcpy(shaderData + parameter->GetOffset(), ParameterValue, parameter->GetSize());
}

RefCountingPtr<RHI::RHIConstantBuffer> MaterialInstance::CreateConstantBuffer(const MaterialShaderMetaType* ShaderMetaType)
{
	if (!ShaderMetaType)
	{
		LE_ASSERT(false)
		return nullptr;
	}

	const ShaderParametersMetadata* metadata = ShaderMetaType->GetShaderParameterMetadata();
	if (!metadata)
	{
		LE_ASSERT(false)
		return nullptr;
	}

	const auto& iterator = Data.find(ShaderMetaType);
	if (iterator == Data.end())
	{
		LE_ASSERT_DESC(false, "Trying to create constant buffer for unallocated shader")
		return nullptr;
	}

	// This will need to be reworked
	RefCountingPtr<RHI::RHIConstantBuffer> newConstantBuffer = RHICreateConstantBuffer(iterator->second, metadata->GetLayout());
	ShaderConstantBuffers[ShaderMetaType] = newConstantBuffer;
	return newConstantBuffer;
}

void MaterialInstance::GetShaderBindings(MeshDrawSingleShaderBindings* ShaderBindings, const Shader* Shader,
                                         const MaterialShaderMetaType* ShaderMetaType, const SceneView* SceneView)
{
	if (!Shader)
	{
		LE_ASSERT(false)
		return;
	}

	if (!SceneView)
	{
		LE_ASSERT(false)
		return;
	}

	if (!ShaderBindings)
	{
		LE_ASSERT(false)
		return;
	}

	RefCountingPtr<RHI::RHIConstantBuffer> constantBuffer = CreateConstantBuffer(ShaderMetaType);
	ShaderBindings->Add(Shader->GetConstantBufferParameter(ShaderMetaType->GetShaderParameterMetadata()), constantBuffer);
	ShaderBindings->AddResources(Shader, constantBuffer);

	ShaderBindings->Add<ViewShaderParametersConstantBuffer>(Shader, SceneView->ConstantBuffer);
}

void MaterialInstance::AllocateShaderParameters(const MaterialShaderMetaType* ShaderMetaType)
{
	if (!ShaderMetaType)
	{
		LE_ASSERT(false)
		return;
	}

	if (Data.contains(ShaderMetaType))
	{
		LE_ASSERT_DESC(false, "Trying to allocate shader parameters for already added shader")
		return;
	}

	const ShaderParametersMetadata* metadata = ShaderMetaType->GetShaderParameterMetadata();
	if (!metadata)
	{
		LE_ASSERT(false)
		return;
	}

	Data[ShaderMetaType] = new uint8[metadata->GetSize()]();
}
}
