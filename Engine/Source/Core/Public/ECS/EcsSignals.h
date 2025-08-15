#pragma once
#include "Misc/Delegate.h"
#include "CoreMinimum.h"

namespace LE
{
template <typename SignalType>
class Sink;

template <typename Callback>
class Signal;

template <typename ReturnType, typename... InputArgs>
class Signal<ReturnType(InputArgs...)>
{
	friend class Sink<Signal<ReturnType(InputArgs...)>>;
	using delegate_type = Delegate<ReturnType(InputArgs...)>;

public:
	using size_type = std::size_t;
	using sink_type = Sink<Signal<ReturnType(InputArgs...)>>;

	Signal() noexcept = default;

	Signal(const Signal& Other) noexcept
		: Listeners(Other.Listeners)
	{
	}

	Signal(Signal&& Other) noexcept
		: Listeners(std::move(Other.Listeners))
	{
	}

	~Signal() = default;

	Signal& operator=(const Signal& Other)
	{
		Listeners = Other.Listeners;
		return *this;
	}

	Signal& operator=(Signal&& Other) noexcept
	{
		std::swap(Other.Listeners);
		return *this;
	}

	size_type Count() const noexcept
	{
		return Listeners.size();
	}

	bool IsEmpty() const noexcept
	{
		return Listeners.empty();
	}

	void Dispatch(InputArgs... Args) const
	{
		for (size_t current = 0; current < Listeners.size(); ++current)
		{
			Listeners[current](Args...);
		}
	}

private:
	std::vector<delegate_type> Listeners;
};

template <typename ReturnType, typename... Args>
class Sink<Signal<ReturnType(Args...)>>
{
	using signal_type = Signal<ReturnType(Args...)>;
	using delegate_type = typename signal_type::delegate_type;

public:
	Sink() noexcept
		: AssociatedSignal(nullptr)
	{
	}

	Sink(signal_type& Signal) noexcept
		: AssociatedSignal(&Signal)
	{
	}

	template<auto Function>
	void Attach()
	{
		Detach<Function>();

		delegate_type delegate{};
		delegate.template Attach<Function>();
		GetSignal().Listeners.push_back(std::move(delegate));
	}

	template<auto Function, typename Type>
	void Attach(Type& Payload)
	{
		Detach<Function>(Payload);

		delegate_type delegate{};
		delegate.template Attach<Function>(Payload);
		GetSignal().Listeners.push_back(std::move(delegate));
	}

	template<auto Function, typename Type>
	void Attach(Type* Payload)
	{
		Detach<Function>(Payload);

		delegate_type delegate{};
		delegate.template Attach<Function>(Payload);
		GetSignal().Listeners.push_back(std::move(delegate));
	}

	template<auto Function>
	void Detach()
	{
		delegate_type delegate{};
		delegate.template Attach<Function>();
		DetachChecked(std::move(delegate));

	}

	template<auto Function, typename Type>
	void Detach(Type& Payload)
	{
		delegate_type delegate{};
		delegate.template Attach<Function>(Payload);
		DetachChecked(std::move(delegate));
	}

	template<auto Function, typename Type>
	void Detach(Type* Payload)
	{
		delegate_type delegate{};
		delegate.template Attach<Function>(Payload);
		DetachChecked(std::move(delegate));
	}

	void Detach(const void* Payload)
	{
		DetachChecked(Payload);
	}

private:
	signal_type& GetSignal()
	{
		LE_ASSERT_DESC(AssociatedSignal != nullptr, "Null pointer to the associated signal")
		return *AssociatedSignal;
	}

	void DetachChecked(delegate_type Listener)
	{
		signal_type& signal = GetSignal();
		for (size_t current = 0; current < signal.Listeners.size(); ++current)
		{
			auto& listener = signal.Listeners[current];
			if (Listener == listener)
			{
				listener = std::move(signal.Listeners.back());
				signal.Listeners.pop_back();
			}
		}
	}

	void DetachChecked(const void* Payload)
	{
		signal_type& signal = GetSignal();
		for (size_t current = 0; current < signal.Listeners.size(); ++current)
		{
			auto& listener = signal.Listeners[current];
			if (Payload == listener.GetPayload())
			{
				listener = std::move(signal.Listeners.back());
				signal.Listeners.pop_back();
			}
		}
	}

private:
	signal_type* AssociatedSignal;
};

template<typename ReturnType, typename ...Args>
Sink(Signal<ReturnType(Args...)>) -> Sink<Signal<ReturnType(Args...)>>;
}
