#pragma once

#include <concepts>
#include "Misc/Traits.h"

template<typename T>
concept Numeric = std::integral<T> or std::floating_point<T>;

template<typename T>
concept Unsigned = std::unsigned_integral<T>;

template<typename T>
concept Alignable = std::is_pointer_v<T> or std::integral<T>;

template<typename T>
concept Identifier = requires
{
	typename LE::IdTraits<T>::ValueType;
	typename LE::IdTraits<T>::IdType;
	typename LE::IdTraits<T>::GenerationType;
};