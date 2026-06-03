#pragma once
#include "Templates/RefCounters.h"

namespace LE
{
class JobScheduler;

class AsyncNode : public RefCountableBase
{
	friend JobScheduler;

public:
	virtual void Execute() = 0;
};
}
