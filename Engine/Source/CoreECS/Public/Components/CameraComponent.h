#pragma once
#include "ECS/EcsComponent.h"

namespace LE
{
class CameraComponent : public EcsComponent
{
public:
	COMPONENT_CLASS("CameraComponent")

	CameraComponent(EntityId OwnerId)
		: EcsComponent(OwnerId)
	{
	}

	float FOV = 90.0f;
};
}
