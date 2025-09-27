#pragma once

#include <chrono>

namespace LE
{
	class Clock
	{
		using ClockClass = std::chrono::system_clock;
	public:
		using TimePoint = std::chrono::time_point<ClockClass>;

		static TimePoint Now();

		static void StartFrame();
		static float GetElapsedSeconds();
		static float GetSecondsBetween(const TimePoint& Begin, const TimePoint& Stop);
		static float GetSecondsFrom(const TimePoint& Begin);

		static float GetMsBetween(const TimePoint& Begin, const TimePoint& Stop);
		static float GetMsFrom(const TimePoint& Begin);

		void Start();
		// TODO: change float to something larger
		float GetElapsedTime() const;
		float GetElapsedTimeMs() const;

	private:
		TimePoint StartPoint;
	};

	extern Clock gGlobalClock;
}
