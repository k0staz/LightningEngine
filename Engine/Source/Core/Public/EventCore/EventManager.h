#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "ECS/EcsComponent.h"

namespace LE
{
	using EventType = std::string;

	class Event
	{
	public:
		virtual ~Event() = default;
		virtual EventType GetEventType() const = 0;
	};

#define EVENT_TYPE(event_type)                  \
	static EventType GetStaticEventType()		\
	{											\
		return EventType(event_type);			\
	}											\
	EventType GetEventType() const override	    \
	{											\
		return GetStaticEventType();			\
	}

	template<typename EventClass>
	using EventListener = std::function<void(const EventClass& e)>;

	class IEventListenerWrapper
	{
	public:
		virtual ~IEventListenerWrapper() = default;

		void Execute(const Event& Event)
		{
			Call(Event);
		}

		virtual std::string GetName() const = 0;

	protected:
		virtual void Call(const Event& Event) = 0;
	};

	template<typename EventClass>
	class EventListenerWrapper final : public IEventListenerWrapper
	{
	public:
		explicit EventListenerWrapper(const EventListener<EventClass>& HandlerFunction)
			: Listener(HandlerFunction)
			, Name(HandlerFunction.target_type().name())
		{}

	protected:
		void Call(const Event& Event) override
		{
			if (Event.GetEventType() == EventClass::GetStaticEventType())
			{
				Listener(static_cast<const EventClass&>(Event));
			}
		}

		std::string GetName() const override { return Name; }

	private:
		EventListener<EventClass> Listener;
		const std::string Name;
	};

	class EventManager
	{
	public:
		EventManager() = default;
		EventManager(const EventManager&) = delete;
		const EventManager& operator=(const EventManager&) = delete;

		void Shutdown();  

		void ListenToEvent(EventType EventType, std::unique_ptr<IEventListenerWrapper>&& Listener);
		void Unsubscribe(EventType EventType, const std::string& ListenerName);
		void QueueEvent(std::unique_ptr<Event>&& Event);
		void DispatchEvents();

	private:
		void DispatchEvent(const Event& Event);

		std::vector<std::unique_ptr<Event>> EventQueue; // TODO: Circular bufffer?
		std::unordered_map<EventType, std::vector<std::unique_ptr<IEventListenerWrapper>>> Listeners;
		
	};

	extern EventManager gEventManager;

	template<typename EventClass>
	inline void ListenToEvent(const EventListener<EventClass>& Callback)
	{
		std::unique_ptr<IEventListenerWrapper> wrapper = std::make_unique<EventListenerWrapper<EventClass>>(Callback);
		gEventManager.ListenToEvent(EventClass::GetStaticEventType(), std::move(wrapper));
	}

	template<typename EventClass>
	inline void Unsubscribe(const EventListener<EventClass>& Callback)
	{
		const std::string listenerName = Callback.target_type().name();
		gEventManager.Unsubscribe(EventClass::GetStaticEventType(), listenerName);
	}

	inline void QueueEvent(std::unique_ptr<Event>&& QueuedEvent)
	{
		gEventManager.QueueEvent(std::forward<std::unique_ptr<Event>>(QueuedEvent));
	}
}