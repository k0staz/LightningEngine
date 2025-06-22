#include "Time/Clock.h"

namespace LE
{
	Clock gGlobalClock;

	Clock::TimePoint Clock::Now()
	{
		return ClockClass::now();
	}

	float Clock::GetSecondsBetween(const TimePoint& Begin, const TimePoint& Stop)
	{
		const std::chrono::duration<float> diff = Stop - Begin;
		return diff.count();
	}

	float Clock::GetSecondsFrom(const TimePoint& Begin)
	{
		const TimePoint now = Now();
		return GetSecondsBetween(Begin, now);
	}

	float Clock::GetMsBetween(const TimePoint& Begin, const TimePoint& Stop)
	{
		return GetSecondsBetween(Begin, Stop) * 1000.f;
	}

	float Clock::GetMsFrom(const TimePoint& Begin)
	{
		return GetSecondsFrom(Begin) * 1000.f;
	}

	void Clock::Start()
	{
		StartPoint = Now();
	}

	float Clock::GetElapsedTime() const
	{
		return GetSecondsFrom(StartPoint);
	}

	float Clock::GetElapsedTimeMs() const
	{
		return GetMsFrom(StartPoint);
	}
}

