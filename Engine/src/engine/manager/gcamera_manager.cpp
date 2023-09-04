#include "internal/engine/manager/gcamera_manager.h"

GCameraManager::GCameraManager()
{
	m_defaultPositioner = nullptr;
	m_selectedPositioner = m_defaultPositioner;

}

bool GCameraManager::init()
{
	return true;
}

void GCameraManager::destroy()
{
	if (m_defaultPositioner != nullptr)
	{
		delete m_defaultPositioner;
	}
}

void GCameraManager::update(float deltaTime)
{
	m_selectedPositioner->update(deltaTime);
}

ICameraPositioner* GCameraManager::get_current_positioner()
{
	return m_selectedPositioner;
}
