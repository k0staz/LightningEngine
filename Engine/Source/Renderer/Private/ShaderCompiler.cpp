#include "ShaderCompiler.h"

#include "Shader.h"
#include "FileManager/FileManager.h"
#include "MeshConverters/MeshConverter.h"
#include "Misc/Paths.h"

namespace LE::Renderer
{
static ShaderCompilerModule* gShaderCompilerModule = nullptr;

void RegisterShaderCompilerModule(ShaderCompilerModule* CompilerModule)
{
	gShaderCompilerModule = CompilerModule;
}

ShaderCompilerModule* GetShaderCompilerModule()
{
	if (!gShaderCompilerModule)
	{
		LE_ERROR("Shader Compiler was not registered");
	}

	return gShaderCompilerModule;
}

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

	return GetShaderCompilerModule()->CompileShader(compilerInput, Result);
}
}
