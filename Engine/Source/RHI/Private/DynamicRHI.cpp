#include "DynamicRHI.h"

#include "D3D11DynamicRHI.h"

namespace LE::RHI
{
DynamicRHI* gDynamicRHI = nullptr;
static DynamicRHIModule* gDynamicRHIModule = nullptr;

void InitRHI()
{
	if (gDynamicRHI)
	{
		return;
	}

	gDynamicRHI = CreateDynamicRHI();
	gDynamicRHI->Init();
}

void DeleteRHI()
{
	if (gDynamicRHI)
	{
		gDynamicRHI->Shutdown();
		delete gDynamicRHI;
		gDynamicRHI = nullptr;
	}

	if (gDynamicRHIModule)
	{
		delete gDynamicRHIModule;
		gDynamicRHIModule = nullptr;
	}
}


static DynamicRHIModule* GetDynamicRHIModule()
{
	if (gDynamicRHIModule)
	{
		return gDynamicRHIModule;
	}

	// For now just create D3D11
	gDynamicRHIModule = new D3D11::D3D11DynamicRHIModule();

	return gDynamicRHIModule;
}

DynamicRHI* CreateDynamicRHI()
{
	DynamicRHI* dynamicRHI = nullptr;

	if (DynamicRHIModule* dynamicRHIModule = GetDynamicRHIModule())
	{
		dynamicRHI = dynamicRHIModule->CreateRHI();
	}

	return dynamicRHI;
}
}
