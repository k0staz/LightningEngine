#pragma once
#include "IGameViewport.h"
#include "Viewport.h"
#include "Templates/RefCounters.h"

namespace LE
{
class GameViewport : public IGameViewport
{
public:

	void Draw() override;

	RefCountingPtr<Renderer::Viewport> Viewport;
};
}
