#include "MeshConverters/MeshConverter.h"

namespace LE::Renderer
{
static Array<MeshConverterType*>* gMeshConverters;


MeshConverterType::MeshConverterType(const char* InConverterName, const char* InAssociatedShaderFile)
	: Name(InConverterName)
	  , ShaderFileName(InAssociatedShaderFile)
{
	GetTypeList()->push_back(this);
}

MeshConverterType::~MeshConverterType()
{
	Array<MeshConverterType*>*& list = GetTypeList();
	const auto& it = std::find(list->begin(), list->end(), this);
	if (it != list->end())
	{
		list->erase(it);
	}
}

Array<MeshConverterType*>*& MeshConverterType::GetTypeList()
{
	if (!gMeshConverters)
	{
		gMeshConverters = new Array<MeshConverterType*>;
	}

	return gMeshConverters;
}

MeshConverterType* MeshConverterType::GetTypeByName(String& Name)
{
	Array<MeshConverterType*>*& list = GetTypeList();
	const auto& it = std::find_if(list->begin(), list->end(), [&Name](MeshConverterType*& element) { return element->Name == Name; });
	if (it != list->end())
	{
		return *it;
	}

	return nullptr;
}
}
