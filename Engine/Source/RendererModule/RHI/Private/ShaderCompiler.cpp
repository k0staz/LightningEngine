#include "ShaderCompiler.h"

#include "slang.h"
#include "slang-com-ptr.h"

#include <array>
#include <ranges>

#include "Core.h"
#include "Log.h"
#include "Misc/Paths.h"
#include "tracy/Tracy.hpp"


namespace LE::RHI
{
void printDiagnostics(slang::IBlob* diagnostics)
{
	if (diagnostics)
	{
		LE_ERROR("{}", static_cast<const char*>(diagnostics->getBufferPointer()));
	}
}

using namespace slang;

ShaderModuleBlob::ShaderModuleBlob(void const* DataIn, uint64 SizeIn)
{
	Data = std::malloc(SizeIn);
	Size = SizeIn;
	std::memcpy(Data, DataIn, SizeIn);
}

ShaderModuleBlob::~ShaderModuleBlob()
{
	if (Data)
	{
		std::free(Data);
	}
}

void ShaderCompiler::Initialize()
{
	SlangGlobalSessionDesc desc = {};
	createGlobalSession(&desc, &SlangGlobalSession);
	CreateSlangSession();
}

void ShaderCompiler::Shutdown()
{
	for (auto& value : LoadedModules | std::views::values)
	{
		value->release();
	}

	LoadedModules.clear();

	if (SlangSession)
	{
		SlangSession->release();
	}

	if (SlangGlobalSession)
	{
		SlangGlobalSession->release();
	}
}

ShaderCompiler::SpecializedProgram ShaderCompiler::CompileProgram(const SpecializedProgramDesc& Desc)
{
	ZoneScopedN("ShaderCompiler: Compile Program");

	Slang::ComPtr<slang::IBlob> diagnostics;
	IModule* module = LoadModule(Desc.ShaderPath);
	if (!module)
	{
		LE_ERROR("Failed to load module");
		return {};
	}

	Slang::ComPtr<slang::IComponentType> composedProgram = nullptr;
	if (!ComposeProgram(module, Desc, composedProgram.writeRef()))
	{
		LE_ERROR("Failed to compose program");
		return {};
	}

	Slang::ComPtr<slang::IComponentType> linkedProgram = nullptr;
	composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());
	printDiagnostics(diagnostics);
	if (!linkedProgram)
	{
		LE_ERROR("Failed to link program");
		return {};
	}

	Slang::ComPtr<ISlangBlob> spirv;
	linkedProgram->getTargetCode(0, spirv.writeRef(), diagnostics.writeRef());
	printDiagnostics(diagnostics);

	ShaderCompiler::SpecializedProgram result;
	result.CompiledModule = new ShaderModuleBlob(spirv->getBufferPointer(), spirv->getBufferSize());

	return result;
}

slang::IModule* ShaderCompiler::LoadModule(const std::string& Path)
{
	ZoneScopedN("ShaderCompiler: Load Module");
	if (LoadedModules.contains(Path))
	{
		return LoadedModules[Path];
	}

	Slang::ComPtr<slang::IBlob> diagnostics;
	IModule* module = SlangSession->loadModule(Path.c_str(), diagnostics.writeRef());
	printDiagnostics(diagnostics);
	if (module)
	{
		LoadedModules[Path] = module;
	}
	return module;
}

bool ShaderCompiler::CreateSlangSession()
{
	std::array<TargetDesc, 2> targets = {};
	targets[0].format = SLANG_SPIRV;
	targets[0].profile = SlangGlobalSession->findProfile("spirv_1_5");

	targets[1].format = SLANG_GLSL;
	targets[1].profile = SlangGlobalSession->findProfile("glsl_460");

	auto slangOptions{std::to_array<slang::CompilerOptionEntry>({{
		slang::CompilerOptionName::EmitSpirvDirectly,
		{slang::CompilerOptionValueKind::Int, 1}
	}})};

	slang::SessionDesc slangSessionDesc{
		.targets{targets.data()},
		.targetCount{static_cast<SlangInt>(targets.size())},
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR,
		.compilerOptionEntries{slangOptions.data()},
		.compilerOptionEntryCount{static_cast<uint32>(slangOptions.size())}
	};

	Path shaderRoot = GetShaderRoot();

	const std::string rootPath = shaderRoot.generic_string();
	const std::string globalsPath = (shaderRoot / "Globals").generic_string();
	const std::string meshPath = (shaderRoot / "Mesh").generic_string();

	const char* searchPaths[] = {rootPath.c_str(), globalsPath.c_str(), meshPath.c_str(), };

	slangSessionDesc.searchPaths = searchPaths;
	slangSessionDesc.searchPathCount = static_cast<uint32>(std::size(searchPaths));

	return SlangGlobalSession->createSession(slangSessionDesc, &SlangSession) == SLANG_OK;
}

bool ShaderCompiler::ComposeProgram(slang::IModule* Module, const SpecializedProgramDesc& Desc,
	slang::IComponentType** ComposedProgramOut)
{
	ZoneScopedN("ShaderCompiler: Compose Program");
	std::vector<Slang::ComPtr<slang::IComponentType>> specializedEntryPoints;

	auto specializeEntryPoint = [&specializedEntryPoints, &Module, &Desc](const std::string& entryPointName)
	{
		slang::IEntryPoint* entryPoint = nullptr;
		Module->findEntryPointByName(entryPointName.c_str(), &entryPoint);
		if (!entryPoint)
		{
			LE_ERROR("Failed to find vertex entry point");
			return false;
		}

		Slang::ComPtr<slang::IComponentType> specializedVertexEp = nullptr;
		if (!SpecializeEntryPoint(entryPoint, Module, Desc, specializedVertexEp.writeRef()))
		{
			return false;
		}

		specializedEntryPoints.push_back(specializedVertexEp);
		return true;
	};

	if (Desc.ShaderStage & RHIShaderStage::Vertex)
	{
		if (!specializeEntryPoint("vertexMain"))
		{
			LE_ERROR("Failed to specialize vertex entry point");
			return false;
		}
	}

	if (Desc.ShaderStage & RHIShaderStage::Fragment)
	{
		if (!specializeEntryPoint("fragmentMain"))
		{
			LE_ERROR("Failed to specialize fragment entry point");
			return false;
		}
	}

	std::vector<IComponentType*> components;
	components.reserve(specializedEntryPoints.size() + 1);
	components.emplace_back(Module);
	for (auto& specializedEntryPoint : specializedEntryPoints)
	{
		components.push_back(specializedEntryPoint.get());
	}


	Slang::ComPtr<IBlob> diagnostics;
	SlangSession->createCompositeComponentType(
		components.data(),
		components.size(),
		ComposedProgramOut,
		diagnostics.writeRef()
	);
	printDiagnostics(diagnostics);
	if (!*ComposedProgramOut)
	{
		LE_ERROR("Failed to compose program");
		return false;
	}

	return true;
}

bool ShaderCompiler::SpecializeEntryPoint(slang::IEntryPoint* EntryPoint, slang::IModule* Module, const SpecializedProgramDesc& Desc,
	slang::IComponentType** SpecializedEntryPointOut)
{
	ZoneScopedN("ShaderCompiler: Specialize Entry Point");
	auto moduleLayout = Module->getLayout();

	auto functionReflection = EntryPoint->getFunctionReflection();
	auto genericContainer = functionReflection->getGenericContainer();
	auto typeParameterCount = genericContainer->getTypeParameterCount();

	std::vector<std::string> pushConstantsInterfaces;
	for (uint32_t j = 0; j < typeParameterCount; ++j)
	{
		auto typeParameter = genericContainer->getTypeParameter(j);
		auto constraintType = genericContainer->getTypeParameterConstraintType(typeParameter, 0);

		auto constraintName = constraintType->getName();
		pushConstantsInterfaces.emplace_back(constraintName);
	}

	if (pushConstantsInterfaces.empty())
	{
		*SpecializedEntryPointOut = EntryPoint;
		return true;
	}

	std::vector<slang::SpecializationArg> specializationArgs;
	specializationArgs.reserve(pushConstantsInterfaces.size());
	for (auto& interface : pushConstantsInterfaces)
	{
		std::string implementationName = "";
		for (const auto& variation : Desc.ModuleSpecializations)
		{
			if (interface == variation.Interface)
			{
				implementationName = variation.ImplementationName;
				break;
			}
		}

		if (implementationName.empty())
		{
			LE_ERROR("No implementation name found for interface: {}", interface);
			return false;
		}

		slang::TypeReflection* implementation = moduleLayout->findTypeByName(implementationName.c_str());
		if (!implementation)
		{
			LE_ERROR("No implementation: {} found for interface: {}", implementationName, interface);
			return false;
		}

		specializationArgs.emplace_back(SpecializationArg::fromType(implementation));
	}

	Slang::ComPtr<IBlob> diagnostics;
	EntryPoint->specialize(
		specializationArgs.data(),
		static_cast<SlangInt>(specializationArgs.size()),
		SpecializedEntryPointOut,
		diagnostics.writeRef());
	printDiagnostics(diagnostics);

	return *SpecializedEntryPointOut;
}
}
