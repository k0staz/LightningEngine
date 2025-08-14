#pragma once

#include <concepts>

#include "ECS/EcsEntity.h"

template<typename T>
concept Numeric = std::integral<T> or std::floating_point<T>;

template<typename T>
concept Unsigned = std::unsigned_integral<T>;

template<typename T>
concept Alignable = std::is_pointer_v<T> or std::integral<T>;

template<typename T>
concept EcsEntityIdentifier = requires
{
	typename LE::EcsEntityTraits<T>::ValueType;
	typename LE::EcsEntityTraits<T>::IdType;
	typename LE::EcsEntityTraits<T>::GenerationType;
};