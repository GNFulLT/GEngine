#include "internal/engine/manager/ginjection_manager.h"

GInjectManagerHelper::GInjectManagerHelper(ManagerTable* table)
{
	m_table = table;
}

void* GInjectManagerHelper::get_manager_spec(ENGINE_MANAGER manager)
{
	if (auto dataI = m_map.find(manager); dataI != m_map.end())
	{
		return dataI->second;
	}
	return nullptr;
}

void GInjectManagerHelper::delete_and_swap(ENGINE_MANAGER id, void* data)
{
	m_table->delete_and_swap(id,data);
}
void* GInjectManagerHelper::swap_and_get_managed(ENGINE_MANAGER mng, void* ptr)
{
	return m_table->swap_and_get_managed(mng, ptr);
}
void GInjectManagerHelper::add_manager_spec(ENGINE_MANAGER id, void* data)
{
	m_map.emplace(id, data);
}
