#pragma once
#include "IWorld.h"

namespace LE
{
namespace Renderer
{
	class RendererModule;
}

class IGameEngine
{
public:
	virtual Renderer::RendererModule* GetRendererModule() = 0;
	virtual IWorld* GetWorld() = 0;
};
}
