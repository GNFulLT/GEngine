#include "internal/engine/manager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "public/platform/window.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/manager/iglogger_manager.h"
#include "engine/manager/ijob_manager.h"
#include "engine/manager/igshader_manager.h"
#include "engine/manager/igresource_manager.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/manager/igpipeline_object_manager.h"
#include "engine/manager/igscene_manager.h"
#include "engine/manager/igscript_manager.h"

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
	assert(false);
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

void* ManagerTable::swap_and_get_managed(ENGINE_MANAGER mng, void* ptr)
{
	auto manager = get_engine_manager_managed(mng);
	
	void* managedPtr;

	switch (mng)
	{
	case ENGINE_MANAGER_WINDOW:
		managedPtr = new GSharedPtr<Window>((Window*)ptr);
		break;
	case ENGINE_MANAGER_GRAPHIC_DEVICE:
		managedPtr = new GSharedPtr<IGVulkanDevice>((IGVulkanDevice*)ptr);
		break;
	case ENGINE_MANAGER_LOGGER:
		managedPtr = new GSharedPtr<IGLoggerManager>((IGLoggerManager*)ptr);
		break;
	case ENGINE_MANAGER_RESOURCE:
		managedPtr = new GSharedPtr<IGResourceManager>((IGResourceManager*)ptr);
		break;
	case ENGINE_MANAGER_JOB:
		managedPtr = new GSharedPtr<IJobManager>((IJobManager*)ptr);
		break;
	case ENGINE_MANAGER_SHADER:
		managedPtr = new GSharedPtr<IGShaderManager>((IGShaderManager*)ptr);
		break;
	case ENGINE_MANAGER_CAMERA:
		managedPtr = new GSharedPtr<IGCameraManager>((IGCameraManager*)ptr);
		break;
	case ENGINE_MANAGER_SCENE:
		managedPtr = new GSharedPtr<IGSceneManager>((IGSceneManager*)ptr);
		break;
	case ENGINE_MANAGER_SCRIPT:
		managedPtr = new GSharedPtr<IGScriptManager>((IGScriptManager*)ptr);
		break;
	default:
		break;
	}

	set_manager(mng, managedPtr);

	return manager;
}

void ManagerTable::delete_and_swap(ENGINE_MANAGER mng, void* ptr)
{
	delete_manager(mng);
	void* managedPtr;
	switch (mng)
	{
	case ENGINE_MANAGER_WINDOW:
		managedPtr = new GSharedPtr<Window>((Window*)ptr);
		break;
	case ENGINE_MANAGER_GRAPHIC_DEVICE:
		managedPtr = new GSharedPtr<IGVulkanDevice>((IGVulkanDevice*)ptr);
		break;
	case ENGINE_MANAGER_LOGGER:
		managedPtr = new GSharedPtr<IGLoggerManager>((IGLoggerManager*)ptr);
		break;
	case ENGINE_MANAGER_RESOURCE:
		managedPtr = new GSharedPtr<IGResourceManager>((IGResourceManager*)ptr);
		break;
	case ENGINE_MANAGER_JOB:
		managedPtr = new GSharedPtr<IJobManager>((IJobManager*)ptr);
		break;
	case ENGINE_MANAGER_SHADER:
		managedPtr = new GSharedPtr<IGShaderManager>((IGShaderManager*)ptr);
		break;
	case ENGINE_MANAGER_CAMERA:
		managedPtr = new GSharedPtr<IGCameraManager>((IGCameraManager*)ptr);
		break;
	case ENGINE_MANAGER_SCENE:
		managedPtr = new GSharedPtr<IGSceneManager>((IGSceneManager*)ptr);
		break;
	case ENGINE_MANAGER_SCRIPT:
		managedPtr = new GSharedPtr<IGScriptManager>((IGScriptManager*)ptr);
		break;
	default:
		break;
	}

	set_manager(mng, managedPtr);

}
void ManagerTable::delete_manager(ENGINE_MANAGER manager)
{
	auto mng = get_engine_manager_managed(manager);
	if (mng == nullptr)
		return;

	switch (manager)
	{
	case ENGINE_MANAGER_WINDOW:
		delete (GSharedPtr<Window>*)mng;
		break;
	case ENGINE_MANAGER_GRAPHIC_DEVICE:
		delete (GSharedPtr<IGVulkanDevice>*)mng;
		break;
	case ENGINE_MANAGER_LOGGER:
		delete (GSharedPtr<IGLoggerManager>*)mng;
		break;
	case ENGINE_MANAGER_RESOURCE:
		delete (GSharedPtr<IGResourceManager>*)mng;
		break;
	case ENGINE_MANAGER_JOB:
		delete (GSharedPtr<IJobManager>*)mng;
		break;
	case ENGINE_MANAGER_SHADER:
		delete (GSharedPtr<IGShaderManager>*)mng;
		break;
	case ENGINE_MANAGER_CAMERA:
		delete (GSharedPtr<IGCameraManager>*)mng;
	case ENGINE_MANAGER_SCENE:
		delete (GSharedPtr<IGSceneManager>*)mng;
		break;
	case ENGINE_MANAGER_SCRIPT:
		delete (GSharedPtr<IGScriptManager>*)mng;
		break;
	default:
		break;
	}

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
	if (auto resource = get_engine_manager_managed(ENGINE_MANAGER_RESOURCE); resource != nullptr)
	{
		delete (GSharedPtr<IGResourceManager>*)resource;
	}
	if (auto job = get_engine_manager_managed(ENGINE_MANAGER_JOB); job != nullptr)
	{
		//X TODO : DELETE LOGGER
		delete (GSharedPtr<IJobManager>*)job;
	}
	if (auto shader = get_engine_manager_managed(ENGINE_MANAGER_SHADER); shader != nullptr)
	{
		//X TODO : DELETE LOGGER
		delete (GSharedPtr<IGShaderManager>*)shader;
	}
	if (auto camera = get_engine_manager_managed(ENGINE_MANAGER_CAMERA); camera != nullptr)
	{
		delete (GSharedPtr<IGCameraManager>*)camera;
	}
	if (auto pipeline = get_engine_manager_managed(ENGINE_MANAGER_PIPELINE_OBJECT); pipeline != nullptr)
	{
		delete (GSharedPtr<IGPipelineObjectManager>*)pipeline;
	}
	if (auto scene = get_engine_manager_managed(ENGINE_MANAGER_SCENE); scene != nullptr)
	{
		delete (GSharedPtr<IGSceneManager>*)scene;
	}
	if (auto script = get_engine_manager_managed(ENGINE_MANAGER_SCRIPT); script != nullptr)
	{
		delete (GSharedPtr<IGScriptManager>*)script;
	}
}
