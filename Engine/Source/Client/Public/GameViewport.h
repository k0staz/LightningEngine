#pragma once
#include "Viewport.h"
#include "Templates/RefCounters.h"

namespace LE
{
class GameViewport
{
public:

	void Draw();

	RefCountingPtr<Renderer::Viewport> Viewport;
};
}
