#pragma once
#include "ShaderCompilerCore.h"

namespace LE::Renderer
{
class MeshConverterType;
class ShaderMetaType;

class ShaderCompilerModule
{
public:
	virtual bool CompileShader(const ShaderCompilerInput& CompilerInput, ShaderCompilerResult& CompilerResult) = 0;
};

void RegisterShaderCompilerModule(ShaderCompilerModule* CompilerModule);
ShaderCompilerModule* GetShaderCompilerModule();

bool CompileShader(const ShaderMetaType* ShaderToCompile, const MeshConverterType* MCToCompileWith, ShaderCompilerResult& Result);
}
