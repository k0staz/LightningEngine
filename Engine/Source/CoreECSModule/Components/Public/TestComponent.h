#pragma once

#include "ECS/EcsComponent.h"

namespace LE
{
struct TestComponent
{
	TestComponent() = default;
};

ECS_REGISTER_COMPONENT(TestComponent, "TestComponent")
}
