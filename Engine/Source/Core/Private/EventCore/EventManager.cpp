#include "EventCore/EventManager.h"

#include "CoreMinimum.h"

namespace LE
{
	EventManager gEventManager;

	void EventManager::Shutdown()
	{
		EventQueue.clear();
		Listeners.clear();
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
					LE_ASSERT_DESC(false, "[Event Manager] Attempting to double register Event Listener");
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

	void EventManager::DispatchEvents()
	{
		for (auto it = EventQueue.begin(); it != EventQueue.end(); ++it)
		{
			DispatchEvent(*it->get());
		}

		EventQueue.clear();
	}

	void EventManager::DispatchEvent(const Event& Event)
	{
		for (auto& listener : Listeners[Event.GetEventType()])
		{
			listener->Execute(Event);
		}
	}
}
