#include "internal/engine/manager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "public/platform/window.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/manager/iglogger_manager.h"
void* ManagerTable::get_engine_manager_managed(ENGINE_MANAGER manager)
{
	if (auto itr = m_manager_map.find((int)manager);itr != m_manager_map.end())
	{
		return itr->second;
	}
	return nullptr;
}

void* ManagerTable::get_engine_manager_raw(ENGINE_MANAGER manager)
{
	if (auto itr = m_manager_map.find((int)manager); itr != m_manager_map.end())
	{
		return itr->second;
	}
	return nullptr;
}

void ManagerTable::set_manager(ENGINE_MANAGER manager, void* pManager)
{
	m_manager_map[(int)manager] = pManager;
}

void ManagerTable::delete_managers()
{
	if (auto window = get_engine_manager_managed(ENGINE_MANAGER_WINDOW); window != nullptr)
	{
		delete (GSharedPtr<Window>*)window;
	}
	//X TODO DELETER WILL BE ADDED
	if (auto graphicDevice = get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);graphicDevice != nullptr)
	{
		delete (GSharedPtr<IGVulkanDevice>*)graphicDevice;
	}
	if (auto logger = get_engine_manager_managed(ENGINE_MANAGER_LOGGER); logger != nullptr)
	{
		//X TODO : DELETE LOGGER
		delete (GSharedPtr<IGLoggerManager>*)logger;

	}
	
}
