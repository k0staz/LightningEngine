#pragma once
#include "ShaderCore.h"

namespace LE::Renderer
{
struct ShaderCompilerResult
{
	ShaderCompilerResult() = default;

	ShaderParameterAllocationMap ParametersMap;
	Array<String> CompilerErrors;
	Array<uint8> Code;
	uint32 CodeSize;
	bool IsSucceeded;
};

struct ShaderCompilerInput
{
	ShaderCompilerInput() = default;

	String FileName;
	String SourceCode;
	String EntryPoint;
	String TargetType;
};


struct ShaderCompilationConfig
{
	Array<String> IncludeStrings;
};
}
