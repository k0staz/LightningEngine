#pragma once

#include "ECS/EcsComponent.h"

#include "Math/Matrix4x4.h"

namespace LE
{
struct TransformComponent
{
	TransformComponent() = default;

	Matrix4x4F Transform;
};

ECS_REGISTER_COMPONENT(TransformComponent, "TransformComponent")
}
