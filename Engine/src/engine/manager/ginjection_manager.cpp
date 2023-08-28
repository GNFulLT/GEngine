#include "internal/engine/manager/ginjection_manager.h"

void* GInjectManagerHelper::get_manager_spec(ENGINE_MANAGER manager)
{
	if (auto dataI = m_map.find(manager); dataI != m_map.end())
	{
		return dataI->second;
	}
	return nullptr;
}

void GInjectManagerHelper::add_manager_spec(ENGINE_MANAGER id, void* data)
{
	m_map.emplace(id, data);
}
