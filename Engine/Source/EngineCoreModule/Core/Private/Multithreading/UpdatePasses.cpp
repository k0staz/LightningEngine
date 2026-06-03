#include "Multithreading/UpdatePasses.h"

namespace LE
{
static std::vector<const UpdatePass*>* gUpdatePassRegistry = nullptr;
static std::unordered_map<UpdatePassType, const UpdatePass*>* gUpdatePassMap = nullptr;

std::vector<const UpdatePass*>& UpdatePass::GetUpdatePasses()
{
   if (!gUpdatePassRegistry)
   {
	   gUpdatePassRegistry = new std::vector<const UpdatePass*>();
   }

   return *gUpdatePassRegistry;
}

const UpdatePass* UpdatePass::GetUpdatePass(UpdatePassType UpdatePass)
{
	auto map = GetUpdatePassMap();
	if (map.contains(UpdatePass))
	{
		return map.at(UpdatePass);
	}

	return nullptr;
}

std::unordered_map<UpdatePassType, const UpdatePass*>& UpdatePass::GetUpdatePassMap()
{
	if (!gUpdatePassMap)
	{
		gUpdatePassMap = new std::unordered_map<UpdatePassType, const UpdatePass*>();
	}

	return *gUpdatePassMap;
}
}
