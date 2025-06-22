#pragma once

#include <concepts>

template<typename T>
concept Numeric = std::integral<T> or std::floating_point<T>;

template<typename T>
concept Alignable = std::is_pointer_v<T> or std::integral<T>;