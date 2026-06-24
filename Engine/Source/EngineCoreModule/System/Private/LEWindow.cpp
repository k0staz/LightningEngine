#include "LEWindow.h"

#include "Core.h"
#include "SDL3/SDL_video.h"

namespace LE
{

LEWindow::LEWindow(void* WindowHandle, const std::string& InName, uint32 InWidth, uint32 InHeight)
	: WindowName(InName), Height(InHeight), Width(InWidth), WindowHandle(WindowHandle)
{
}

LEWindow::~LEWindow()
{
	LE_ASSERT_DESC(!WindowHandle, "Leaked Window. It should be destroyed via System Manager")
}

void LEWindow::Reset()
{
	WindowHandle = nullptr;
	WindowName = "";
	Height = 0;
	Width = 0;
}
}
