#include "DynamicRHI.h"

namespace LE::RHI
{
DynamicRHI* gDynamicRHI = nullptr;

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
}

static DynamicRHIModule* gRegisteredModule = nullptr;

void RegisterRHIModule(DynamicRHIModule* RHIModule)
{
	gRegisteredModule = RHIModule;
}

DynamicRHIModule* GetRHIModule()
{
	return gRegisteredModule;
}

DynamicRHI* CreateDynamicRHI()
{
	DynamicRHI* dynamicRHI = nullptr;

	if (DynamicRHIModule* dynamicRHIModule = GetRHIModule())
	{
		dynamicRHI = dynamicRHIModule->CreateRHI();
	}

	return dynamicRHI;
}
}
