#pragma once
#include "Math.h"

namespace LE::MathUtils
{
constexpr float DegreesToRadians(const float Degrees)
{
    return Degrees * (PI / 180.0f);
}
}
