#pragma once
#include "EngineRegistry.h"

namespace LE::Renderer
{
class RendererModule;
}

namespace LE
{
inline Renderer::RendererModule* GetRendererModule()
{
	return GetGameEngine()->GetRendererModule();
}

inline IWorld* GetWorld()
{
	return GetGameEngine()->GetWorld();
}
}
