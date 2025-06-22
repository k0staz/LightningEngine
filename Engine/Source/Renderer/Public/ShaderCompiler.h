#pragma once
#include "ShaderCompilerCore.h"

namespace LE::Renderer
{
	class MeshConverterType;
	class ShaderMetaType;

	bool CompileShader(const ShaderMetaType* ShaderToCompile, const MeshConverterType* MCToCompileWith, ShaderCompilerResult& Result);
}
