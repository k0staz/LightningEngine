#pragma once
#include <vector>

#include "CoreDefinitions.h"

namespace LE
{
template <typename ElementType>
class Array : public std::vector<ElementType>
{
public:
	using std::vector<ElementType>::vector;

	uint32 Count() const
	{
		return static_cast<uint32>(this->size());
	}

};
}
