#pragma once
#include <type_traits>

#include "Core.h"
#include "Templates/TypeHelpers.h"

namespace LE
{
namespace DelegateInternal
{
	template <typename ReturnType, typename... Args>
	constexpr auto FunctionPointer(ReturnType (*)(Args...)) -> ReturnType(*)(Args...);

	template <typename ReturnType, typename Type, typename... Args, typename Other>
	constexpr auto FunctionPointer(ReturnType (*)(Type, Args...), Other&&) -> ReturnType(*)(Args...);

	template <typename Class, typename ReturnType, typename... Args, typename... Other>
	constexpr auto FunctionPointer(ReturnType (Class::*)(Args...), Other&&...) -> ReturnType(*)(Args...);

	template <typename Class, typename ReturnType, typename... Args, typename... Other>
	constexpr auto FunctionPointer(ReturnType (Class::*)(Args...) const, Other&&...) -> ReturnType(*)(Args...);

	template <typename Class, typename Type, typename... Other, typename = std::enable_if_t<std::is_member_object_pointer_v<Type Class::*>>>
	constexpr auto FunctionPointer(Type Class::*, Other&&...) -> Type(*)();

	template <typename... Type>
	using FunctionPointerType = decltype(FunctionPointer(std::declval<Type>()...));

	template <typename... Class, typename ReturnType, typename... Args>
	constexpr auto IndexSequenceFor(ReturnType (*)(Args...))
	{
		return std::index_sequence_for<Class..., Args...>{};
	}
}


template <typename>
class Delegate;

template <typename ReturnType, typename... Args>
class Delegate<ReturnType(Args...)>
{
	using return_type = std::remove_const_t<ReturnType>;
	using delegate_type = return_type(const void*, Args...);

public:
	using function_type = ReturnType(const void*, Args...);
	using type = ReturnType(Args...);
	using result_type = ReturnType;

	Delegate() noexcept = default;

	bool operator==(const Delegate& OtherDelegate) const noexcept
	{
		return Function == OtherDelegate.Function && Payload == OtherDelegate.Payload;
	}

	template <auto FunctionType>
	void Attach() noexcept
	{
		Payload = nullptr;

		if constexpr (std::is_invocable_r_v<ReturnType, decltype(FunctionType), Args...>)
		{
			Function = [](const void*, Args... InArgs) -> return_type
			{
				return ReturnType(std::invoke(FunctionType, std::forward<Args>(InArgs)...));
			};
		}
		else if constexpr (std::is_member_pointer_v<decltype(FunctionType)>)
		{
			Function = Wrap<FunctionType>(
				DelegateInternal::IndexSequenceFor<TypeListElementType<0, TypeList<Args...>>>(
					DelegateInternal::FunctionPointerType<decltype(FunctionType)>{}));
		}
		else
		{
			Function = Wrap<FunctionType>(DelegateInternal::IndexSequenceFor(DelegateInternal::FunctionPointerType<decltype(FunctionType)>{}));
		}
	}

	template <auto FunctionType, typename Type>
	void Attach(Type& PayloadIn) noexcept
	{
		Payload = &PayloadIn;

		if constexpr (std::is_invocable_r_v<ReturnType, decltype(FunctionType), Type&, Args...>)
		{
			Function = [](const void* payload, Args... args) -> return_type
			{
				Type* curr = static_cast<Type*>(const_cast<TransferConstnessType<void, Type>*>(payload));
				return ReturnType(std::invoke(FunctionType, *curr, std::forward<Args>(args)...));
			};
		}
		else
		{
			Function = Wrap<FunctionType>(
				PayloadIn, DelegateInternal::IndexSequenceFor(DelegateInternal::FunctionPointerType<decltype(FunctionType), Type>{}));
		}
	}

	template <auto FunctionType, typename Type>
	void Attach(Type* PayloadIn) noexcept
	{
		Payload = PayloadIn;

		if constexpr (std::is_invocable_r_v<ReturnType, decltype(FunctionType), Type*, Args...>)
		{
			Function = [](const void* payload, Args... args) -> return_type
			{
				Type* curr = static_cast<Type*>(const_cast<TransferConstnessType<void, Type>*>(payload));
				return ReturnType(std::invoke(FunctionType, curr, std::forward<Args>(args)...));
			};
		}
		else
		{
			Function = Wrap<FunctionType>(
				PayloadIn, DelegateInternal::IndexSequenceFor(DelegateInternal::FunctionPointerType<decltype(FunctionType), Type>{}));
		}
	}

	void Attach(function_type* FunctionIn, const void* PayloadIn = nullptr) noexcept
	{
		LE_ASSERT_DESC(FunctionIn != nullptr, "Null function pointer")
		Payload = PayloadIn;
		Function = FunctionIn;
	}

	void Detach() noexcept
	{
		Payload = nullptr;
		Function = nullptr;
	}

	function_type* GetAttachedFunction() const noexcept
	{
		return Function;
	}

	const void* GetPayload() const noexcept
	{
		return Payload;
	}

	ReturnType operator()(Args... ArgsIn) const
	{
		LE_ASSERT_DESC(static_cast<bool>(*this), "Uninitialized delegate usage")
		return Function(Payload, std::forward<Args>(ArgsIn)...);
	}

	explicit operator bool() const noexcept
	{
		return !(Function == nullptr);
	}

private:
	template <auto FreeFunction, std::size_t... Index>
	auto Wrap(std::index_sequence<Index...>) noexcept
	{
		return [](const void*, Args... ArgsIn) -> return_type
		{
			[[maybe_unused]] const auto arguments = std::forward_as_tuple(std::forward<Args>(ArgsIn)...);
			[[maybe_unused]] constexpr auto argsOffset = !std::is_invocable_r_v<
					ReturnType, decltype(FreeFunction), TypeListElementType<Index, TypeList<Args...>>...>
				* (sizeof...(Args) - sizeof...(Index));
			return static_cast<ReturnType>(std::invoke(
				FreeFunction,
				std::forward<TypeListElementType<Index + argsOffset, TypeList<Args...>>>(std::get<Index + argsOffset>(ArgsIn))...));
		};
	}

	template <auto Method, typename ObjectType, std::size_t... Index>
	auto Wrap(ObjectType&, std::index_sequence<Index...>) noexcept
	{
		return [](const void* Object, Args... ArgsIn)-> return_type
		{
			ObjectType* object = static_cast<ObjectType*>(const_cast<TransferConstnessType<void, ObjectType>*>(Object));
			[[maybe_unused]] const auto arguments = std::forward_as_tuple(std::forward<Args>(ArgsIn)...);
			[[maybe_unused]] constexpr auto argsOffset = !std::is_invocable_r_v<
					ReturnType, decltype(Method), ObjectType&, TypeListElementType<Index, TypeList<Args...>>...>
				* (sizeof...(Args) - sizeof...(Index));
			return static_cast<ReturnType>(std::invoke(
				Method,
				*object,
				std::forward<TypeListElementType<Index + argsOffset, TypeList<Args...>>>(std::get<Index + argsOffset>(ArgsIn))...));
		};
	}

	template <auto Method, typename ObjectType, std::size_t... Index>
	auto Wrap(ObjectType*, std::index_sequence<Index...>) noexcept
	{
		return [](const void* Object, Args... ArgsIn)-> return_type
		{
			ObjectType* object = static_cast<ObjectType*>(const_cast<TransferConstnessType<void, ObjectType>*>(Object));
			[[maybe_unused]] const auto arguments = std::forward_as_tuple(std::forward<Args>(ArgsIn)...);
			[[maybe_unused]] constexpr auto argsOffset = !std::is_invocable_r_v<
					ReturnType, decltype(Method), ObjectType*, TypeListElementType<Index, TypeList<Args...>>...>
				* (sizeof...(Args) - sizeof...(Index));
			return static_cast<ReturnType>(std::invoke(
				Method,
				object,
				std::forward<TypeListElementType<Index + argsOffset, TypeList<Args...>>>(std::get<Index + argsOffset>(ArgsIn))...));
		};
	}

private:
	const void* Payload{};
	function_type* Function;
};

template<typename ReturnType, typename ...Args>
bool operator!=(const Delegate<ReturnType(Args...)>& Lhs, const Delegate<ReturnType(Args...)>& Rhs) noexcept
{
	return !(Lhs == Rhs);
}

template<auto>
struct ConnectArgType
{
	explicit ConnectArgType() = default;
};

template<auto Function>
Delegate(ConnectArgType<Function>) -> Delegate<std::remove_pointer_t<DelegateInternal::FunctionPointerType<decltype(Function)>>>;

template<auto Method, typename Type>
Delegate(ConnectArgType<Method>, Type&&) -> Delegate<std::remove_pointer_t<DelegateInternal::FunctionPointerType<decltype(Method), Type>>>;

template<typename ReturnType, typename ...Args>
Delegate(ReturnType(*)(const void*, Args...), const void* = nullptr) -> Delegate<ReturnType(Args...)>;
}
