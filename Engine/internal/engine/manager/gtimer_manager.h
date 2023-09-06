#ifndef GTIMER_MANAGER_H
#define GTIMER_MANAGER_H

#include <chrono>

class GTimerManager
{
public:
	GTimerManager();

	bool calc_fps(int& fps);

	double calculate_delta_time();
private:
	std::chrono::nanoseconds m_delta;

	std::chrono::steady_clock::time_point m_lastDelta;
	std::chrono::steady_clock::time_point m_firstDelta;

	float m_lastTime = 0;
	int m_frameCountInSecond = 0;
};

#endif // GTIMER_MANAGER_H