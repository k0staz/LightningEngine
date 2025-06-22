#pragma once

#include "CoreDefinitions.h"

namespace LE
{
template <typename T>
concept RefCounterInterface = requires(T* Ptr)
{
	Ptr->AddRef();
	Ptr->Release();
};

class RefCountableBase
{
public:
	RefCountableBase() = default;
	virtual ~RefCountableBase() = default;

	RefCountableBase(const RefCountableBase&) = delete;
	RefCountableBase& operator=(const RefCountableBase&) = delete;

	uint32 AddRef() const
	{
		uint32 newValue = RefsNum.fetch_add(1, std::memory_order_acquire) + 1;
		return newValue;
	}

	uint32 Release() const
	{
		uint32 newValue = RefsNum.fetch_sub(1, std::memory_order_acquire) - 1;
		if (newValue == 0)
		{
			delete this;
		}
		return newValue;
	}

	uint32 GetRefCount() const
	{
		return RefsNum.load(std::memory_order_acquire);
	}

private:
	mutable std::atomic_uint RefsNum = {0};
};

template <RefCounterInterface PointedType>
class RefCountingPtr
{
public:
	typedef PointedType* PointerType;

	RefCountingPtr()
		: Pointer(nullptr)
	{
	}

	RefCountingPtr(PointerType InPtr)
		: Pointer(InPtr)
	{
		InternalAddRef();
	}

	RefCountingPtr(const RefCountingPtr& OtherCopy)
	{
		Pointer = OtherCopy.Pointer;
		InternalAddRef();
	}

	template <RefCounterInterface OtherCopyType>
	RefCountingPtr(const RefCountingPtr<OtherCopyType>& OtherCopy)
	{
		Pointer = static_cast<PointerType>(OtherCopy.Pointer);
		InternalAddRef();
	}

	RefCountingPtr(RefCountingPtr&& OtherMove)
	{
		Pointer = OtherMove.Pointer;
		OtherMove.Pointer = nullptr;
	}

	template <RefCounterInterface OtherMoveType>
	RefCountingPtr(RefCountingPtr<OtherMoveType>&& OtherMove)
	{
		Pointer = static_cast<PointerType>(OtherMove.Pointer);
		OtherMove.Pointer = nullptr;
	}

	~RefCountingPtr()
	{
		InternalRelease();
	}

	RefCountingPtr& operator=(PointerType InPtr)
	{
		if (Pointer == InPtr)
		{
			return *this;
		}

		InternalRelease();
		Pointer = InPtr;
		InternalAddRef();
		return *this;
	}

	RefCountingPtr& operator=(const RefCountingPtr& OtherCopy)
	{
		return *this = OtherCopy.Pointer;
	}

	template <RefCounterInterface OtherCopyType>
	RefCountingPtr& operator=(const RefCountingPtr<OtherCopyType>& OtherCopy)
	{
		return *this = OtherCopy.GetPointer();
	}

	RefCountingPtr& operator=(RefCountingPtr&& OtherMove)
	{
		if (this == &OtherMove)
		{
			return *this;
		}

		InternalRelease();
		Pointer = static_cast<PointerType>(OtherMove.Pointer);
		OtherMove.Pointer = nullptr;

		return *this;
	}

	template <RefCounterInterface OtherMoveType>
	RefCountingPtr& operator=(RefCountingPtr<OtherMoveType>&& OtherMove)
	{
		InternalRelease();
		Pointer = static_cast<PointerType>(OtherMove.Pointer);
		OtherMove.Pointer = nullptr;

		return *this;
	}

	PointerType operator->() const
	{
		return Pointer;
	}

	bool operator==(const RefCountingPtr& Other) const
	{
		return GetPointer() == Other.GetPointer();
	}

	bool operator==(const PointerType Other) const
	{
		return GetPointer() == Other;
	}

	operator PointerType() const
	{
		return Pointer;
	}

	PointerType GetPointer() const
	{
		return Pointer;
	}

	PointerType* GetInitPointer()
	{
		InternalRelease();
		Pointer = nullptr;
		return &Pointer;
	}

	bool IsValid() const
	{
		return Pointer != nullptr;
	}

	uint32 GetRefCount() const
	{
		uint32 result = 0;
		if (Pointer)
		{
			result = Pointer->GetRefCount();
			//LE_ASSERT(result > 0)
		}

		return result;
	}

	void Swap(RefCountingPtr& Other)
	{
		PointerType* oldPtr = Pointer;
		Pointer = Other.Pointer;
		Other.Pointer = oldPtr;
	}

	void Release()
	{
		*this = nullptr;
	}

private:
	void InternalAddRef()
	{
		if (Pointer)
		{
			Pointer->AddRef();
		}
	}

	void InternalRelease()
	{
		if (Pointer)
		{
			Pointer->Release();
		}
	}

private:
	PointerType Pointer;

	template <RefCounterInterface OtherType>
	friend class RefCountingPtr;
};
}
