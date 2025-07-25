#pragma once
#include "ShaderCompiler.h"

namespace LE::Renderer
{
struct ShaderCompilerResult;
struct ShaderCompilationConfig;
struct ShaderCompilerInput;
}

namespace LE::D3D11
{
class D3D11ShaderCompilerModule : public Renderer::ShaderCompilerModule
{
public:
	bool CompileShader(const Renderer::ShaderCompilerInput& CompilerInput, Renderer::ShaderCompilerResult& CompilerResult) override;
};

static D3D11ShaderCompilerModule gD3D11ShaderCompiler;

// This is temp, once we have more backends we need to make them DLLs and load at runtime
inline void UseD3D11ShaderCompiler()
{
	RegisterShaderCompilerModule(&gD3D11ShaderCompiler);
}
}
