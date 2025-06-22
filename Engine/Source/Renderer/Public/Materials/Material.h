#pragma once
#include "CoreMinimum.h"
#include "RenderDefines.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Templates/NonCopyable.h"

namespace LE::Renderer
{
class SceneView;
class MeshDrawSingleShaderBindings;

class MaterialShaderMetaType : public ShaderMetaType
{
public:
	MaterialShaderMetaType(const char* InName, const char* InSourceFileName, const char* InMainFunctionName, const char* InMaterialName,
	                       RHI::ShaderType InShaderType, RenderPassType InRenderPassType,
	                       CreateCompiledType InCreateCompiledRef, const ShaderParametersMetadata* InParameterMetadata);

	String GetMaterialName() const { return MaterialName; }
	RenderPassType GetRenderPassType() const { return RenderPass; }

	// TODO: Add a method for providing exposed parameters

private:
	String MaterialName;
	RenderPassType RenderPass;
};


struct ResolvedMaterialShaderMapping
{
	RefCountingPtr<ShaderMapping> VertexShaderMapping;
	RefCountingPtr<ShaderMapping> PixelShaderMapping;
};

class Material
{
public:
	using ShaderSet = MaterialShaderMetaType* [static_cast<uint8>(RHI::ShaderType::Count)];
	using MaterialRegistry = Map<String, Material*>;

	Material(const String& Name)
		: MaterialName(Name)
	{}

	static MaterialRegistry& GetMaterialRegistry();
	static Material* GetMaterialByName(const String& Name);

	String GetName() const { return MaterialName; }
	const ShaderSet* GetShaderSetForRenderPass(const RenderPassType RenderPass) const;

	const Map<RenderPassType, ShaderSet>& GetShaders() const { return RenderPassShaders; }

	ResolvedMaterialShaderMapping GetShaderMappings(const RenderPassType RenderPass, MeshConverterType* MCType) const;

private:
	String MaterialName;
	Map<RenderPassType, ShaderSet> RenderPassShaders;
	// TODO: Add a list of reflected (user exposed) shader parameters

	friend MaterialShaderMetaType;
};

class MaterialInstance : public NonCopyable, public RenderResource
{
public:
	MaterialInstance(const Material* InMaterial);
	~MaterialInstance() override;

	void SetParameter(const MaterialShaderMetaType* ShaderMetaType, const String& ParameterName, const uint8* ParameterValue);

	RefCountingPtr<RHI::RHIConstantBuffer> CreateConstantBuffer(const MaterialShaderMetaType* ShaderMetaType);

	void GetShaderBindings(MeshDrawSingleShaderBindings* ShaderBindings, const Shader* Shader, const MaterialShaderMetaType* ShaderMetaType, const SceneView* SceneView);

	const Material* GetMaterial() const { return Material; }

private:
	void AllocateShaderParameters(const MaterialShaderMetaType* ShaderMetaType);

private:
	Map<const MaterialShaderMetaType*, uint8*> Data;
	Map<const MaterialShaderMetaType*, RefCountingPtr<RHI::RHIConstantBuffer>> ShaderConstantBuffers;
	const Material* Material;
};


#define DECLARE_MATERIAL_PASS_VARIANT(ShaderName) \
	public: \
	using Shader::Shader;	\
	static MaterialShaderMetaType& StaticGetMetaType(); \
	static Shader* CreateCompiledShader(const CompiledShaderInitializer& Initializer) \
	{ \
		return new ShaderName(Initializer); \
	} \
	static void RegisterMetaType();

#define REGISTER_MATERIAL_PASS_VARIANT(ShaderClass, SourceFilePath, MainFunction, RHIShaderType, MaterialName, RenderPassType) \
	inline MaterialShaderMetaType& ShaderClass::StaticGetMetaType()	\
	{ \
		static MaterialShaderMetaType StaticMetaType( \
			#ShaderClass, \
			SourceFilePath, \
			MainFunction, \
			MaterialName, \
			RHIShaderType, \
			RenderPassType, \
			ShaderClass::CreateCompiledShader, \
			ShaderClass::GetRootParametersMetadata()); \
		return StaticMetaType; \
	} \
	void ShaderClass::RegisterMetaType() \
	{ \
		static ShaderMetaTypeRegistration MetaTypeRegistration{FunctionRef<ShaderMetaType&()>{ShaderClass::StaticGetMetaType}}; \
	} 

#define BEGIN_MATERIAL_CONSTANT_BUFFER(BufferName) \
	static inline const ShaderParametersMetadata* GetMaterialParametersMetadata() { return BufferName::Descriptor::GetParametersMetadata(); } \
	BEGIN_CONSTANT_BUFFER(BufferName)
}
