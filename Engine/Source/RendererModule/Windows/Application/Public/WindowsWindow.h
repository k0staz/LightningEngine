#pragma once
#include "Application/SystemWindow.h"

#include <windows.h>

namespace LE::Windows
{
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class WindowsWindow : public SystemWindow
{
public:
	void Init(const WindowDescription& InDescription, HINSTANCE InHInstance);

	void Show() override;
	void PushToFront() override;
	void SetInFocus() override;
	void* GetSystemWindowHandle() const override;

private:
	HWND Hwnd;
};
}
