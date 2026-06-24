#pragma once
#include "Templates/RefCounters.h"

namespace LE
{
class LEWindow : public RefCountableBase
{
public:
	LEWindow(void* WindowHandle, const std::string& InName, uint32 InWidth, uint32 InHeight);
	~LEWindow() override;
	
	void* GetWindowNativeHandle() const { return WindowHandle; }
	uint32 GetHeight() const { return Height; }
	uint32 GetWidth() const { return Width; }
	const std::string& GetName() const { return WindowName; }
private:
	void Reset();
	std::string WindowName;
	uint32 Height = 0;
	uint32 Width = 0;
	void* WindowHandle = nullptr;

	friend class SystemManager;
};
}
