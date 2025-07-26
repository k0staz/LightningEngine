#include "MaterialShaderAutoRegistration.h"
#include "RenderPasses/BaseRenderPass.h"

namespace LE::Renderer
{
    void RegisterAllMaterialShader()
    {
    BaseVS::RegisterMetaType();
    BasePS::RegisterMetaType();
    }
}
