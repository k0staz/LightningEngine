#pragma once

#include "ECS/EcsSystem.h"
#include "EventCore/EventManager.h"

namespace LE
{
	class TestSystem : public EcsSystem
	{
	public:
		void Initialize() override;
		void Update(const float DeltaSeconds) override;
		void Shutdown() override;

		/*void OnArchetypeMatched(const ArchetypeMatched& Event);
		void OnArchetypeUnmatched(const ArchetypeUnmatched& Event);*/

	private:
		/*EventListener<ArchetypeMatched> ArchetypeMatchListener;
		EventListener<ArchetypeUnmatched> ArchetypeUnmatchListener;*/
	};
}