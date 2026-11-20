#pragma once
#include "Misc/Paths.h"
#include "Service/ServiceBase.h"

namespace LE
{
class MountRegistry : public ServiceBase
{
public:
	void Initialize() override;
	void Shutdown() override;

private:
	std::unordered_map<std::string, Path> Mounts;
};

REGISTER_SERVICE_TYPE(MountRegistry, "MountRegistry")
}