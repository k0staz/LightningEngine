#pragma once
#include "ECS/EcsSystem.h"

namespace LE
{
class RenderSystem : public EcsSystem
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
