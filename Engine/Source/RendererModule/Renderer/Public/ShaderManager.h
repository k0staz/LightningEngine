#pragma once
#include "Shader.h"

namespace LE::Renderer
{
class ShaderMapping : public RenderResource
{
public:
	ShaderMapping()
		: LogicalShader(nullptr)
		  , ShaderRHI(nullptr)
	{
	}

	ShaderMapping(Shader* LShader, RHI::RHIShader* RhiShader)
		: LogicalShader(LShader)
		, ShaderRHI(RhiShader)
	{
	}

	RefCountingPtr<Shader> GetLogicalShader() const { return LogicalShader; }
	RefCountingPtr<RHI::RHIShader> GetShaderRHI() const { return ShaderRHI; }

	void InitRHI(RenderCommandList& CommandList) override;

private:
	RefCountingPtr<Shader> LogicalShader;
	RefCountingPtr<RHI::RHIShader> ShaderRHI;
};

struct ShaderVariantKey
{
	ShaderVariantKey()
		: MCVariant(0)
		  , MaterialVariant(0)
	{
	}

	uint32 MCVariant;
	uint32 MaterialVariant;

	bool operator==(const ShaderVariantKey& Rhs) const
	{
		return MCVariant == Rhs.MCVariant && MaterialVariant == Rhs.MaterialVariant;
	}
};

struct ShaderVariantKeyHasher {
	size_t operator()(const ShaderVariantKey& Key) const
	{
		return (static_cast<LE::uint64>(Key.MaterialVariant) << 32 | static_cast<LE::uint64>(Key.MCVariant));
	}
};

class ShaderMap
{
public:
	ShaderMap()
		: ShaderType(nullptr)
	{
	}

	const ShaderMetaType* GetShaderMetaType() const { return ShaderType; }
	RefCountingPtr<ShaderMapping> GetShaderMapping(const ShaderVariantKey& VariantKey) const;
	void AddShaderMapping(const ShaderVariantKey& VariantKey, RefCountingPtr<ShaderMapping> NewMapping);

private:
	ShaderMetaType* ShaderType;
	Map<ShaderVariantKey, RefCountingPtr<ShaderMapping>, ShaderVariantKeyHasher> VariantMap;
};

struct ShaderLookUpKey
{
	ShaderLookUpKey()
		: ShaderType(nullptr)
		  , MCType(nullptr)
	{
	}

	ShaderMetaType* ShaderType;
	MeshConverterType* MCType;
	ShaderVariantKey VariantKey;
};

class ShaderManager
{
public:
	ShaderManager(ShaderManager&&) = delete;
	ShaderManager(const ShaderManager&) = delete;
	void operator=(const ShaderManager&) = delete;
	void operator=(ShaderManager&&) = delete;

	~ShaderManager() = default;

	static ShaderManager* Get();

	static RefCountingPtr<ShaderMapping> GetShaderMapping(const ShaderLookUpKey& LookUpKey);

	RefCountingPtr<ShaderMapping> CompileShaderVariant(const ShaderLookUpKey& LookUpKey);

private:
	Map<ShaderMetaType*, ShaderMap> ShaderMaps;

private:
	ShaderManager() = default;
	static ShaderManager* GShaderManager;
};
}