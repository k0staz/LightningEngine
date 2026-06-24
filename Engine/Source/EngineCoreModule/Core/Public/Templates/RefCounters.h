#pragma once

#include "CoreDefinitions.h"

namespace LE
{
class RefCountableBase
{
public:
	RefCountableBase() = default;
	virtual ~RefCountableBase() = default;

	RefCountableBase(RefCountableBase&& other) noexcept
	: RefsNum(other.RefsNum.load(std::memory_order_relaxed))
	{
		other.RefsNum.store(0, std::memory_order_relaxed);
	}
	RefCountableBase& operator=(RefCountableBase&& other) noexcept
	{
		if (this != &other)
		{
			RefsNum.store(other.RefsNum.load(std::memory_order_relaxed), std::memory_order_relaxed);
			other.RefsNum.store(0, std::memory_order_relaxed);
		}
		return *this;
	}
	
	RefCountableBase(const RefCountableBase&) = delete;
	RefCountableBase& operator=(const RefCountableBase&) = delete;

	uint32 AddRef() const
	{
		uint32 newValue = RefsNum.fetch_add(1, std::memory_order_relaxed) + 1;
		return newValue;
	}

	uint32 Release() const
	{
		uint32 newValue = RefsNum.fetch_sub(1, std::memory_order_acq_rel) - 1;
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

template<class T>
concept DerivesFromRefBase = requires(std::remove_cv_t<std::remove_reference_t<T>>*Ptr)
{
	static_cast<RefCountableBase const*>(Ptr);
};

template <typename T>
concept RefCounterInterface = DerivesFromRefBase<T> || requires(T* Ptr)
{
	Ptr->AddRef();
	Ptr->Release();
};

template <typename PointedType>
class RefCountingPtrBase
{
public:
	typedef PointedType* PointerType;

	RefCountingPtrBase()
		: Pointer(nullptr)
	{
	}

	RefCountingPtrBase(PointerType InPtr)
		: Pointer(InPtr)
	{
		InternalAddRef();
	}

	RefCountingPtrBase(const RefCountingPtrBase& OtherCopy)
	{
		Pointer = OtherCopy.Pointer;
		InternalAddRef();
	}

	template <RefCounterInterface OtherCopyType>
	RefCountingPtrBase(const RefCountingPtrBase<OtherCopyType>& OtherCopy)
	{
		Pointer = static_cast<PointerType>(OtherCopy.Pointer);
		InternalAddRef();
	}

	RefCountingPtrBase(RefCountingPtrBase&& OtherMove)
	{
		Pointer = OtherMove.Pointer;
		OtherMove.Pointer = nullptr;
	}

	template <RefCounterInterface OtherMoveType>
	RefCountingPtrBase(RefCountingPtrBase<OtherMoveType>&& OtherMove)
	{
		Pointer = static_cast<PointerType>(OtherMove.Pointer);
		OtherMove.Pointer = nullptr;
	}

	~RefCountingPtrBase()
	{
		InternalRelease();
	}

	RefCountingPtrBase& operator=(PointerType InPtr)
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

	RefCountingPtrBase& operator=(const RefCountingPtrBase& OtherCopy)
	{
		return *this = OtherCopy.Pointer;
	}

	template <RefCounterInterface OtherCopyType>
	RefCountingPtrBase& operator=(const RefCountingPtrBase<OtherCopyType>& OtherCopy)
	{
		return *this = OtherCopy.GetPointer();
	}

	RefCountingPtrBase& operator=(RefCountingPtrBase&& OtherMove)
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
	RefCountingPtrBase& operator=(RefCountingPtrBase<OtherMoveType>&& OtherMove)
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

	bool operator==(const RefCountingPtrBase& Other) const
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

	void Swap(RefCountingPtrBase& Other)
	{
		PointerType* oldPtr = Pointer;
		Pointer = Other.Pointer;
		Other.Pointer = oldPtr;
	}
	
	PointerType Detach() noexcept
	{
		PointerType oldPtr = Pointer;
		Pointer = nullptr;
		return oldPtr;
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

	template <typename OtherType>
	friend class RefCountingPtrBase;
};

template <RefCounterInterface PointedType>
class RefCountingPtr : public RefCountingPtrBase<PointedType>
{
	using RefCountingPtrBase<PointedType>::RefCountingPtrBase;
};

template<typename T>
class ExternalRefCountingPtr : public RefCountingPtrBase<T>
{
	using RefCountingPtrBase<T>::RefCountingPtrBase;
};
}

namespace std
{
template<typename T>
struct hash<LE::RefCountingPtr<T>>
{
	size_t operator()(const LE::RefCountingPtr<T>& p) const noexcept
	{
		return hash<typename LE::RefCountingPtr<T>::PointerType>{}(p.GetPointer());
	}
};
}
