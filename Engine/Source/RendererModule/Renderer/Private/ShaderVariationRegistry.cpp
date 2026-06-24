#include "ShaderVariationRegistry.h"

namespace LE::Renderer
{
void ShaderVariationRegistry::Initialize()
{
}

void ShaderVariationRegistry::Shutdown()
{
    RenderContributorShaderFiles.clear();
    ShaderPassFiles.clear();
}

void ShaderVariationRegistry::RegisterRenderContributorShaderFile(RenderContributorTypeId ContributorTypeId, std::string_view ShaderFile,
                                                                  std::string_view InterfaceName, std::string_view ImplementationName)
{
    RHI::ShaderCompiler::VariationData contributorData;
    contributorData.ShaderFile = ShaderFile;
    contributorData.Interface = InterfaceName;
    contributorData.ImplementationName = ImplementationName;
    RenderContributorShaderFiles.emplace(ContributorTypeId, contributorData);
}

void ShaderVariationRegistry::RegisterShaderPassFile(ShaderPassTypeId ShaderPassTypeId, std::string_view Path, RHI::RHIShaderStage Stages,
                                                     RHI::RHIShaderStage PushConstantStages, uint64 PushConstantSize)
{
    ShaderPassVariation ShaderPassDesc;
    ShaderPassDesc.Path = Path;
    ShaderPassDesc.Stages = Stages;
    ShaderPassDesc.PushConstantStages = PushConstantStages;
    ShaderPassDesc.PushConstantSize = PushConstantSize;
    ShaderPassFiles.emplace(ShaderPassTypeId, ShaderPassDesc);
}

RHI::ShaderCompiler::VariationData ShaderVariationRegistry::GetRenderContributorData(RenderContributorTypeId ContributorTypeId) const
{
    static RHI::ShaderCompiler::VariationData empty;
    return RenderContributorShaderFiles.contains(ContributorTypeId) ? RenderContributorShaderFiles.at(ContributorTypeId) : empty;
}

std::string_view ShaderVariationRegistry::GetShaderPassFile(ShaderPassTypeId ShaderPassTypeId) const
{
    static std::string_view EmptyStringView;
    return ShaderPassFiles.contains(ShaderPassTypeId) ? ShaderPassFiles.at(ShaderPassTypeId).Path : EmptyStringView;
}

ShaderPassVariation ShaderVariationRegistry::GetShaderPassVariation(ShaderPassTypeId ShaderPassTypeId) const
{
    static ShaderPassVariation empty;
    return ShaderPassFiles.contains(ShaderPassTypeId) ? ShaderPassFiles.at(ShaderPassTypeId) : empty;
}
}
