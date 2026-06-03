#pragma once
#include "CoreMinimum.h"


namespace LE
{
class ResourceArrayInterface
{
public:
	virtual ~ResourceArrayInterface()
	{
	}

	virtual const void* GetResourceData() const = 0;
	virtual uint32 GetResourceDataSize() const = 0;
	virtual void Discard() = 0;
	virtual bool IsStatic() const = 0;
	virtual bool GetAllowCPUAccess() const = 0;
	virtual void SetAllowCPUAccess(bool InAllowsCPUAccess) = 0;
};

template <typename ElementType>
class ResourceArray : public ResourceArrayInterface
                      , public std::vector<ElementType>
{
	using ParentArrayType = std::vector<ElementType>;

public:
	using Super = ParentArrayType;

	ResourceArray(bool InNeedCPUAccess = false)
		: Super()
		  , NeedsCPUAccess(InNeedCPUAccess)
	{
	}

	const void* GetResourceData() const override
	{
		return &(*this)[0];
	}

	uint32 GetResourceDataSize() const override
	{
		return static_cast<uint32>(this->size() * sizeof(ElementType));
	}

	void Discard() override
	{
		this->clear();
	}

	bool GetAllowCPUAccess() const override
	{
		return NeedsCPUAccess;
	}

	void SetAllowCPUAccess(bool InAllowsCPUAccess) override
	{
		NeedsCPUAccess = InAllowsCPUAccess;
	}

	ResourceArray& operator=(const Super& Other)
	{
		Super::operator=(Other);
		return *this;
	}

	bool IsStatic() const override
	{
		return false;
	}

private:
	bool NeedsCPUAccess;
};
}
