#pragma once
#include "Containers/String.h"
#include "Templates/RefCounters.h"

namespace LE
{
struct WindowDescription
{
	uint32 DesiredScreenPositionX;
	uint32 DesiredScreenPositionY;

	uint32 DesiredWidth;
	uint32 DesiredHeight;

	String Title;
};

class SystemWindow : public RefCountableBase
{
public:
	SystemWindow() = default;

	virtual void Show() = 0;
	virtual void PushToFront() = 0;
	virtual void SetInFocus() = 0;

	virtual void* GetSystemWindowHandle() const = 0;

	const WindowDescription& GetDescription() const { return Description; }


protected:
	WindowDescription Description;
};
}
