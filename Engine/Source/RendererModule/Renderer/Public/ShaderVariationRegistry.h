#pragma once
#include "ShaderCompiler.h"
#include "RenderContributors/RenderContributerCore.h"
#include "Service/ServiceBase.h"
#include "ShaderPass/ShaderPass.h"
#include "Templates/Alignment.h"


namespace LE::Renderer
{
class ShaderVariationRegistry : public ServiceBase
{
public:
    void Initialize() override;
    void Shutdown() override;

    template <DerivedFromRenderContributor Type>
    void RegisterRenderContributorShaderFile()
    {
        RegisterRenderContributorShaderFile(RenderContributorTypeIdGetter<Type>::Value, RenderContributorTypeIdGetter<Type>::Path,
                                            RenderContributorTypeIdGetter<Type>::InterfaceValue,
                                            RenderContributorTypeIdGetter<Type>::TypeName);
    }

    template <ShaderPassType Type>
    void RegisterShaderPassFile()
    {
        RegisterShaderPassFile(ShaderPassGetter<Type>::Value, ShaderPassGetter<Type>::Path, ShaderPassGetter<Type>::Stages,
                               ShaderPassGetter<Type>::PushConstantStages, sizeof(TStructAligned<typename Type::PushConstants>));
    }

    void RegisterRenderContributorShaderFile(RenderContributorTypeId ContributorTypeId, std::string_view ShaderFile,
                                             std::string_view InterfaceName,
                                             std::string_view ImplementationName);
    void RegisterShaderPassFile(ShaderPassTypeId ShaderPassTypeId, std::string_view Path, RHI::RHIShaderStage Stages,
                                RHI::RHIShaderStage PushConstantStages, uint64 PushConstantSize);
    RHI::ShaderCompiler::VariationData GetRenderContributorData(RenderContributorTypeId ContributorTypeId) const;

    template <DerivedFromRenderContributor Type>
    RHI::ShaderCompiler::VariationData GetRenderContributorData() const
    {
        return GetRenderContributorData(RenderContributorTypeIdGetter<Type>::Value);
    }

    std::string_view GetShaderPassFile(ShaderPassTypeId ShaderPassTypeId) const;
    ShaderPassVariation GetShaderPassVariation(ShaderPassTypeId ShaderPassTypeId) const;

private:
    std::unordered_map<RenderContributorTypeId, RHI::ShaderCompiler::VariationData> RenderContributorShaderFiles;
    std::unordered_map<ShaderPassTypeId, ShaderPassVariation> ShaderPassFiles;
};
}

namespace LE
{
REGISTER_SERVICE_TYPE(Renderer::ShaderVariationRegistry, "ShaderVariationRegistry")
}
