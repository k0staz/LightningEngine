#pragma once

#include "RHIDefinitions.h"
#include "Service/ServiceBase.h"
#include "Templates/RefCounters.h"

class ISlangBlob;

namespace slang
{
struct IModule;
struct IEntryPoint;
struct IComponentType;
struct IGlobalSession;
struct ISession;
typedef ISlangBlob IBlob;
}

namespace LE::RHI
{
class ShaderModuleBlob : public RefCountableBase
{
public:
	ShaderModuleBlob() = default;
	ShaderModuleBlob(void const* DataIn, uint64 SizeIn);
	~ShaderModuleBlob() override;

	void* GetData() const { return Data; }
	uint64 GetSize() const { return Size; }

private:
	void* Data = nullptr;
	uint64 Size = 0;
};

class ShaderCompiler : public ServiceBase
{
public:
	void Initialize() override;
	void Shutdown() override;

	struct SpecializedProgram
	{
		RefCountingPtr<ShaderModuleBlob> CompiledModule;
	};

	struct VariationData
	{
		std::string_view ShaderFile;
		std::string_view Interface;
		std::string_view ImplementationName;
	};
	
	struct SpecializedProgramDesc
	{
		std::string ShaderPath;
		RHI::RHIShaderStage ShaderStage;
		std::vector<VariationData> ModuleSpecializations;
	};

	// TODO: This should be made async, as it takes a LOT OF TIME
	SpecializedProgram CompileProgram(const SpecializedProgramDesc& Desc);
	
private:
	slang::IModule* LoadModule(const std::string& Path);
	bool CreateSlangSession();
	bool ComposeProgram(slang::IModule* Module, const SpecializedProgramDesc& Desc, slang::IComponentType** ComposedProgramOut);
	static bool SpecializeEntryPoint(slang::IEntryPoint* EntryPoint, slang::IModule* Module, const SpecializedProgramDesc& Desc, slang::IComponentType** SpecializedEntryPointOut);
private:
	slang::IGlobalSession* SlangGlobalSession = nullptr;
	slang::ISession* SlangSession = nullptr;
	std::unordered_map<std::string, slang::IModule*> LoadedModules;
};
}

namespace LE
{
REGISTER_SERVICE_TYPE(RHI::ShaderCompiler, "ShaderCompiler")
}