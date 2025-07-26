#pragma once

namespace LE
{
namespace Renderer
{
	struct SceneViewInfo;
}

class IWorld
{
public:
	virtual void SetPrimaryViewInfo(const Renderer::SceneViewInfo& ViewInfo) = 0;
	virtual const Renderer::SceneViewInfo& GetPrimaryViewInfo() const = 0;
};
}
