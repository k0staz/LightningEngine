#pragma once

#include <concepts>

template<typename T>
concept Numeric = std::integral<T> or std::floating_point<T>;