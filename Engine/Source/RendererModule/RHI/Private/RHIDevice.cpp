#include "RHIDevice.h"

namespace LE::RHI
{
static RHIDevice* g_RenderDevice = nullptr;


RHIDevice* RHIDevice::Get()
{
	return g_RenderDevice;
}

void RHIDevice::Register(RHIDevice* DeviceImplementation)
{
	if (g_RenderDevice)
	{
		LE_ASSERT_DESC(false, "Double registration of render devices")
		return;
	}
	g_RenderDevice = DeviceImplementation;
}
}
