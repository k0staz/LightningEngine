#include "EventCore/EventManager.h"

#include "CoreMinimum.h"
#include "EventCore/Events/ArchetypeEvents.h"

namespace LE
{
	EventManager gEventManager;

	namespace EventLocal
	{
		void ListenToCoreEvent(ComponentMask EventType, std::unique_ptr<IEventListenerWrapper>&& Listener, std::unordered_map<LE::ComponentMask, std::vector<std::unique_ptr<IEventListenerWrapper>>>& Listeners)
		{
			auto listenersIt = Listeners.find(EventType);
			if (listenersIt != Listeners.end())
			{
				std::vector<std::unique_ptr<IEventListenerWrapper>>& listeners = listenersIt->second;
				for (auto& it : listeners)
				{
					if (it->GetName() == Listener->GetName())
					{
						LE_ASSERT(false, "[Event Manager] Attempting to double register Event Listener");
						return;
					}
				}

				listeners.emplace_back(std::move(Listener));
			}
			else
			{
				Listeners[EventType].emplace_back(std::move(Listener));
			}
		}
	}

	void EventManager::Shutdown()
	{
		EventQueue.clear();
		Listeners.clear();
		CoreEventQueue.clear();
		ArchetypeMatchedListeners.clear();
		ArchetypeUnmatchedListeners.clear();
		ArchetypeChangeListeners.clear();
	}
	void EventManager::ListenToEvent(EventType EventType, std::unique_ptr<IEventListenerWrapper>&& Listener)
	{
		auto listenersIt = Listeners.find(EventType);
		if (listenersIt != Listeners.end())
		{
			std::vector<std::unique_ptr<IEventListenerWrapper>>& listeners = listenersIt->second;
			for (auto& it : listeners)
			{
				if (it->GetName() == Listener->GetName())
				{
					LE_ASSERT(false, "[Event Manager] Attempting to double register Event Listener");
					return;
				}
			}

			listeners.emplace_back(std::move(Listener));
		}
		else
		{
			Listeners[EventType].emplace_back(std::move(Listener));
		}
	}

	void EventManager::ListenToArchetypeMatchedEvent(ComponentMask Archetype, const EventListener<ArchetypeMatched>& Callback)
	{
		std::unique_ptr<IEventListenerWrapper> wrapper = std::make_unique<EventListenerWrapper<ArchetypeMatched>>(Callback);
		EventLocal::ListenToCoreEvent(Archetype, std::move(wrapper), ArchetypeMatchedListeners);
	}

	void EventManager::ListenToArchetypeUnmatchedEvent(ComponentMask Archetype, const EventListener<ArchetypeUnmatched>& Callback)
	{
		std::unique_ptr<IEventListenerWrapper> wrapper = std::make_unique<EventListenerWrapper<ArchetypeUnmatched>>(Callback);
		EventLocal::ListenToCoreEvent(Archetype, std::move(wrapper), ArchetypeUnmatchedListeners);
	}

	void EventManager::ListenToArchetypeChangeEvent(ComponentMask Archetype, const EventListener<ArchetypeChange>& Callback)
	{
		std::unique_ptr<IEventListenerWrapper> wrapper = std::make_unique<EventListenerWrapper<ArchetypeChange>>(Callback);
		EventLocal::ListenToCoreEvent(Archetype, std::move(wrapper), ArchetypeChangeListeners);
	}

	void EventManager::Unsubscribe(EventType EventType, const std::string& ListenerName)
	{
		auto listenersIt = Listeners.find(EventType);
		if (listenersIt != Listeners.end())
		{
			std::vector<std::unique_ptr<IEventListenerWrapper>>& listeners = listenersIt->second;
			for (auto it = listeners.begin(); it != listeners.end(); ++it)
			{
				if (it->get()->GetName() == ListenerName)
				{
					listeners.erase(it);
					return;
				}
			}
		}
	}
	void EventManager::QueueEvent(std::unique_ptr<Event>&& Event)
	{
		EventQueue.emplace_back(std::move(Event));
	}

	void EventManager::QueueCoreEvent(std::unique_ptr<ArchetypeEvent>&& Event)
	{
		CoreEventQueue.emplace_back(std::move(Event));
	}

	void EventManager::DispatchEvents()
	{
		for (auto it = EventQueue.begin(); it != EventQueue.end(); ++it)
		{
			DispatchEvent(*it->get());
		}

		EventQueue.clear();

		DispatchCoreEvents();
	}

	void EventManager::DispatchCoreEvents()
	{
		// TODO: We need some profiling here
		// We can also group events per archetype in the queue to avoid checking for the same archetype for every event of the same archetype
		for (auto it = CoreEventQueue.begin(); it != CoreEventQueue.end(); ++it)
		{
			const ArchetypeEvent& event = *it->get();
			std::unordered_map<ComponentMask, std::vector<std::unique_ptr<IEventListenerWrapper>>>* listeners = nullptr;

			if (event.GetEventType() == ArchetypeMatched::GetStaticEventType())
			{
				listeners = &ArchetypeMatchedListeners;
			}
			else if (event.GetEventType() == ArchetypeUnmatched::GetStaticEventType())
			{
				listeners = &ArchetypeUnmatchedListeners;
			}
			else if (event.GetEventType() == ArchetypeChange::GetStaticEventType())
			{
				listeners = &ArchetypeChangeListeners;
			}
			else
			{
				continue;
			}

			for (auto listener = listeners->begin(); listener != listeners->end(); ++listener)
			{
				if ((listener->first & event.Archetype) == listener->first)
				{
					for (auto& listener : listener->second)
					{
						listener->Execute(event);
					}
				}
			}
		}

		CoreEventQueue.clear();
	}

	void EventManager::DispatchEvent(const Event& Event)
	{
		for (auto& listener : Listeners[Event.GetEventType()])
		{
			listener->Execute(Event);
		}
	}
}
