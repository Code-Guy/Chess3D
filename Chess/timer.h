#ifndef _TIMER_H
#define _TIMER_H

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__unix__) || defined(unix) || defined(__unix)
#define __PLATFORM_UNIX__
#include <sys/time.h>
#define stopwatch timeval
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define __PLATFORM_WINDOWS__
#define NOMINMAX // required to stop windows.h messing up std::min
#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <windows.h>
#define stopwatch LARGE_INTEGER
#else
#define __PLATFORM_OTHER__
#include <ctime>
#define stopwatch clock_t
#endif

#include <string>

class Timer
{
public:
	Timer();
	void Start();
	void Stop();
	double GetElapsedSeconds();

private:
	stopwatch begin;
	stopwatch end;
	bool isRunning;

#ifdef __PLATFORM_WINDOWS__
	DWORD mStartTick;
	LONGLONG mLastTime;
	LARGE_INTEGER mFrequency;
	DWORD mTimerMask;
#endif

	void SetBeginTime();
	void SetEndTime();
};

#endif //_TIMER_H