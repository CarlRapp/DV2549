#ifndef TIMER_H
#define TIMER_H
#include <SDL/SDL.h>

namespace Utility
{ 
	class Timer
	{
	public:
		Timer();
		~Timer();

		int GetFPS();
		int GetAverageFPS();
		float GetDeltaTime();
		float GetAverageDeltaTime();

		void Tick();
		
		void Reset();
		float GetMinDeltaTime();
		float GetMaxDeltaTime();
		
	private:
		static const unsigned int m_numberOfSamples = 60;

		float m_nrSampleDivided;
		float m_countsPerSecond;
		Uint64 m_frameTimes[m_numberOfSamples];
		unsigned int m_currentIndex = 0;
		Uint64 m_timeStamp;
	};
}





#endif