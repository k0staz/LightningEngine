#include "RenderCore.h"

namespace LE::Renderer
{
std::atomic<LE::uint64> CurrentFrame{0};
uint64 GetCurrentRenderFrame()
{
	return CurrentFrame.load(std::memory_order_acquire);
}

void IncrementCurrentRenderFrame()
{
	CurrentFrame.fetch_add(1, std::memory_order_relaxed);
}
}

