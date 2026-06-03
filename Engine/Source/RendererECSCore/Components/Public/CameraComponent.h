#pragma once
#include "ECS/EcsComponent.h"

namespace LE
{
struct CameraComponent
{
	CameraComponent() = default;

	float FOV = 90.0f;
};

ECS_REGISTER_COMPONENT(CameraComponent, "CameraComponent")
}
