#pragma once
#include "GameEngine.h"

namespace LE::Renderer
{
class RendererModule;
}

namespace LE
{
inline Renderer::RendererModule* GetRendererModule()
{
	return &gGameEngine.RendererModule;
}

inline World* GetWorld()
{
	return gGameEngine.GameWorld;
}
}
