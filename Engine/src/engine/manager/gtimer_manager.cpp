#include "internal/engine/manager/gtimer_manager.h"

GTimerManager::GTimerManager()
{
	m_delta = {};
}

bool GTimerManager::calc_fps(int& fps)
{
	static int frameCount = 0;
	frameCount++;
	auto delFloat = std::chrono::time_point_cast<std::chrono::milliseconds>(m_lastDelta).time_since_epoch();
	if ((delFloat.count()) - m_lastTime > 1000.f)
	{
		m_frameCountInSecond = frameCount;
		frameCount = 0;
		m_lastTime = delFloat.count();
		fps = m_frameCountInSecond;
		return true;
	}
	return false;
}

double GTimerManager::calculate_delta_time()
{
	m_lastDelta =  std::chrono::steady_clock::now();
	m_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(m_lastDelta - m_firstDelta);
	m_firstDelta = m_lastDelta;
	return (m_delta.count()/1000000000.f);
}
