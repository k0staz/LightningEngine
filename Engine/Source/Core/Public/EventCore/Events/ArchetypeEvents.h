#pragma once
#include "EventCore/EventManager.h"

#include <bitset>
#include "ECS/EcsComponent.h"


namespace LE
{
	class ArchetypeEvent : public Event
	{
	public:
		ComponentMask Archetype;
		EntityId EntityId;
	};
	class ArchetypeMatched : public ArchetypeEvent
	{
	public:
		EVENT_TYPE("ArchetypeMatched")
	};

	class ArchetypeUnmatched : public ArchetypeEvent
	{
	public:
		EVENT_TYPE("ArchetypeUnmatched")
	};

	class ArchetypeChange: public ArchetypeEvent
	{
	public:
		EVENT_TYPE("ArchetypeChange")
	};
}