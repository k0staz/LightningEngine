#include "ShaderParameterTypeDescriptors.h"

namespace LE::Renderer
{
static Array<const GlobalConstantBufferRegistration*>* gConstantBufferRegistrations = nullptr;

Array<const GlobalConstantBufferRegistration*>& GlobalConstantBufferRegistration::GetInstances()
{
	if (!gConstantBufferRegistrations)
	{
		gConstantBufferRegistrations = new Array<const GlobalConstantBufferRegistration*>();
	}

	return *gConstantBufferRegistrations;
}
}
