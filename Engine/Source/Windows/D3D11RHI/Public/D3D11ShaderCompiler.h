#pragma once

namespace LE::Renderer
{
struct ShaderCompilerResult;
struct ShaderCompilationConfig;
struct ShaderCompilerInput;
}

namespace LE::D3D11
{
bool CompileShaderD3D11(const Renderer::ShaderCompilerInput& CompilerInput, Renderer::ShaderCompilerResult& CompilerResult);
}
