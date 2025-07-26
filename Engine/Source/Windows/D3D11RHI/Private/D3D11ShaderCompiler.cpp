#include "D3D11ShaderCompiler.h"

#include <Windows.h>

#include "Misc/Paths.h"

#pragma comment(lib, "d3dcompiler.lib")

#include <d3dcompiler.h>

#include "Core.h"
#include "CoreDefinitions.h"
#include "ShaderCompilerCore.h"
#include "Containers/String.h"
#include "Templates/RefCounters.h"

typedef HRESULT (WINAPI*pD3DReflect)
(__in_bcount(SrcDataSize) LPCVOID pSrcData,
 __in SIZE_T SrcDataSize,
 __in REFIID pInterface,
 __out void** ppReflector);

#define DEFINE_GUID_FOR_COMPILER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	static constexpr GUID name = {l, w1, w2, {b1, b2, b3, b4, b5, b6, b7, b8 } }

DEFINE_GUID_FOR_COMPILER(IID_ID3D11ShaderReflectionForCurrentCompiler, 0x8d536ca1, 0x0cca, 0x4956, 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84);

namespace LE::D3D11
{
class CompilerFunctions
{
public:
	static pD3DCompile GetCompile() { return GetInstance().Compile; }
	static pD3DReflect GetReflect() { return GetInstance().Reflect; }

private:
	CompilerFunctions()
	{

		std::filesystem::path compilerPath = GetEngineRoot();
		compilerPath /= "3rdParty/Windows/DirectX/d3dcompiler_47.dll";
		CompilerDLL = LoadLibrary(compilerPath.c_str());
		if (!CompilerDLL)
		{
			LE_ASSERT_DESC(false, "Failed to load D3D11 compiler's dll")
		}

		Compile = (pD3DCompile)(void*)(GetProcAddress(CompilerDLL, "D3DCompile"));
		Reflect = (pD3DReflect)(void*)(GetProcAddress(CompilerDLL, "D3DReflect"));
	}

	static CompilerFunctions& GetInstance()
	{
		static CompilerFunctions instance;
		return instance;
	}

private:
	HMODULE CompilerDLL = nullptr;
	pD3DCompile Compile = nullptr;
	pD3DReflect Reflect = nullptr;
};

void BuildParameterAllocationMap(ID3D11ShaderReflection* Reflection, Renderer::ShaderParameterAllocationMap& AllocationMap)
{
	D3D11_SHADER_DESC shaderDesc;
	Reflection->GetDesc(&shaderDesc);
	for (uint32 index = 0; index < shaderDesc.BoundResources; ++index)
	{
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		Reflection->GetResourceBindingDesc(index, &bindDesc);
		LE_INFO("Found parameters with name {}", bindDesc.Name);

		Renderer::ShaderReflectedParameterType type = Renderer::ShaderReflectedParameterType::Num;
		switch (bindDesc.Type)
		{
		case D3D_SIT_SAMPLER:
			type = Renderer::ShaderReflectedParameterType::Sampler;
			break;
		case D3D_SIT_TEXTURE:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
		case D3D_SIT_RTACCELERATIONSTRUCTURE:
			type = Renderer::ShaderReflectedParameterType::ReadOnlyView;
			break;
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			type = Renderer::ShaderReflectedParameterType::ReadWriteView;
			break;
		case D3D_SIT_CBUFFER:
			type = Renderer::ShaderReflectedParameterType::ConstantBuffer;
			break;
		default:
			LE_WARN("Unknown or unsupported parameter type");
			continue;
		}

		AllocationMap.AddParameterAllocation(bindDesc.Name, static_cast<uint16>(bindDesc.BindPoint), 0, static_cast<uint16>(bindDesc.BindCount), type);
	}
}

bool D3D11ShaderCompilerModule::CompileShader(const Renderer::ShaderCompilerInput& CompilerInput,
	Renderer::ShaderCompilerResult& CompilerResult)
{
	HRESULT result = S_OK;
	RefCountingPtr<ID3DBlob> shader;
	RefCountingPtr<ID3DBlob> errors;

	if (pD3DCompile compileFunc = CompilerFunctions::GetCompile())
	{
		result = compileFunc(
			CompilerInput.SourceCode.c_str(),
			CompilerInput.SourceCode.size(),
			CompilerInput.FileName.c_str(),
			nullptr,
			nullptr,
			CompilerInput.EntryPoint.c_str(),
			CompilerInput.TargetType.c_str(),
			0,
			0,
			shader.GetInitPointer(),
			errors.GetInitPointer());
	}
	else
	{
		CompilerResult.CompilerErrors.emplace_back("Failed to get compiler's DLL");
		result = E_FAIL;
	}

	if (void* errorBuffer = errors ? errors->GetBufferPointer() : nullptr)
	{
		String errorsString(static_cast<char*>(errorBuffer));
		StringUtils::ParseString(errorsString, "\n", CompilerResult.CompilerErrors);
	}

	if (result >= 0)
	{
		if (pD3DReflect reflectFunc = CompilerFunctions::GetReflect())
		{
			CompilerResult.IsSucceeded = true;
			const void* code = shader->GetBufferPointer();
			const size_t codeSize = shader->GetBufferSize();
			CompilerResult.Code.assign(static_cast<const uint8*>(code), static_cast<const uint8*>(code) + codeSize);
			CompilerResult.CodeSize = static_cast<uint32>(codeSize);

			RefCountingPtr<ID3D11ShaderReflection> reflection;
			result = reflectFunc(shader->GetBufferPointer(), shader->GetBufferSize(), IID_ID3D11ShaderReflectionForCurrentCompiler, reinterpret_cast<void**>(reflection.GetInitPointer()));
			if (result < 0)
			{
				LE_ERROR("Failed to get reflection from D3D11Reflect");
				return false;
			}

			BuildParameterAllocationMap(reflection, CompilerResult.ParametersMap);
		}
	}
	else
	{
		CompilerResult.IsSucceeded = false;
		CompilerResult.CompilerErrors.emplace_back("Failed to get shader reflection function from compiler DLL");
		result = E_FAIL;
	}

	return result >= 0;
}
}
