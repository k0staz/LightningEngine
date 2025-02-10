#pragma once

#include "ECS/EcsComponent.h"

namespace LE
{
	class TransformComponent : public EcsComponent
	{
	public:
		COMPONENT_CLASS("TransformComponent")
		
		TransformComponent(EntityId OwnerId)
			: EcsComponent(OwnerId)
		, x(0), y(0), z(0){}

		int x;
		int y;
		int z;
	};
}