#pragma once

#include "ECS/EcsComponent.h"

#include "Math/Matrix4x4.h"

namespace LE
{
	class TransformComponent : public EcsComponent
	{
	public:
		COMPONENT_CLASS("TransformComponent")

		TransformComponent(EntityId OwnerId)
			: EcsComponent(OwnerId)
			  , Transform(Matrix4x4F::Identity())
		{
		}

		Matrix4x4F Transform;
	};
}
