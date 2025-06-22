#include "WindowsWindow.h"

#include "Core.h"
#include "CoreDefinitions.h"


namespace LE::Windows
{
LRESULT WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	// Check if the window is being destroyed.
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

	// Check if the window is being closed.
	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

	default:
		{
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
	}
}

void WindowsWindow::Init(const WindowDescription& InDescription, HINSTANCE InHInstance)
{
	// This will need to be moved to a common place (Application Class) for all windows at some point
	LPCWSTR name = L"LightningEngine";

	{
		WNDCLASSEX wc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = InHInstance;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = name;
		wc.cbSize = sizeof(WNDCLASSEX);

		ATOM result = RegisterClassEx(&wc);
		if (result == 0)
		{
			DWORD err = GetLastError();
			LE_ASSERT_DESC(false, "RegisterClassEx failed: %d", err);
		}
	}


	Description = InDescription;

	int32 windowX = Description.DesiredScreenPositionX;
	int32 windowY = Description.DesiredScreenPositionY;
	int32 width = Description.DesiredWidth;
	int32 height = Description.DesiredHeight;

	uint32 windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;

	SetLastError(0);
	Hwnd = CreateWindowEx(WS_EX_APPWINDOW, name, name, windowStyle, windowX, windowY, width, height, nullptr, nullptr,
	                      InHInstance, nullptr);
	if (!Hwnd)
	{
		DWORD err = GetLastError();
		LE_ASSERT_DESC(false, "Error when creating Windows' Window: %d", err)
	}
}

void WindowsWindow::Show()
{
	::ShowWindow(Hwnd, SW_SHOW);
}

void WindowsWindow::PushToFront()
{
	::SetForegroundWindow(Hwnd);
}

void WindowsWindow::SetInFocus()
{
	::SetFocus(Hwnd);
}

void* WindowsWindow::GetSystemWindowHandle() const
{
	return Hwnd;
}
}
