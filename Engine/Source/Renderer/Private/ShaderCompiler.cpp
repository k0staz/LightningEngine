#include "ShaderCompiler.h"

#include "D3D11ShaderCompiler.h"
#include "Shader.h"
#include "FileManager/FileManager.h"
#include "MeshConverters/MeshConverter.h"
#include "Misc/Paths.h"

namespace LE::Renderer
{
bool CompileShader(const ShaderMetaType* ShaderToCompile, const MeshConverterType* MCToCompileWith, ShaderCompilerResult& Result)
{
	ShaderCompilerInput compilerInput;
	Path shaderPath = GetEngineRoot() / "Shaders";
	shaderPath /= ShaderToCompile->GetSourceFileName();

	compilerInput.FileName = ShaderToCompile->GetSourceFileName();
	compilerInput.EntryPoint = ShaderToCompile->GetMainFunctionName();
	compilerInput.TargetType = ShaderToCompile->GetShaderType() == RHI::ShaderType::Vertex ? "vs_5_0" : "ps_5_0";
	compilerInput.SourceCode = LoadShaderFile(shaderPath);

	// TODO: We don't do preprocessing now
	//ShaderCompilationConfig config;
	//MCToCompileWith->ModifyCompilationConfigFunction(config);

	return D3D11::CompileShaderD3D11(compilerInput, Result);
}
}
