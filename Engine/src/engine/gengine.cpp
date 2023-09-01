#include "engine/gengine.h"
#include "engine/globals.h"
#include "public/platform/window.h"

#include "internal/engine/rendering/vulkan/vulkan_device.h"
#include "internal/engine/rendering/vulkan/vulkan_app.h"
#include <cassert>
#include "internal/engine/manager_table.h"
#include "internal/engine/rendering/vulkan/vulkan_main_viewport.h"
#include "internal/engine/manager/glogger_manager.h"
#include "internal/engine/manager/gresource_manager.h"

#include "engine/rendering/vulkan/vulkan_memory.h"
#include "internal/engine/rendering/vulkan/vulkan_queue.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "internal/engine/rendering/vulkan/vulkan_swapchain.h"
#include "internal/engine/rendering/vulkan/gvulkan_offscreen_viewport.h"
#include "internal/engine/manager/ginjection_manager.h"
#include "internal/engine/manager/gshader_manager.h"
#include "internal/engine/manager/gjob_manager.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static uint32_t SwapchainImageCount = 3;

static GVulkanDevice* s_device;
static GLoggerManager* s_logger;
static GResourceManager* s_resourceManager;
static GEngine* s_engine;
static GJobManager* s_jobManager;
static ManagerTable* s_managerTable;

GEngine::GEngine()
{
	m_window = create_default_window();
	m_impl = nullptr;

	//X TODO : USE GDNEWD
	m_vulkanApp = GSharedPtr<IGVulkanApp>(new GVulkanApp());
	s_managerTable = gdnew(ManagerTable);
	m_managerTable = s_managerTable;
	auto dev = new GVulkanDevice(m_vulkanApp);
	s_logger = new GLoggerManager();
	s_resourceManager = new GResourceManager();
	s_jobManager = new GJobManager();

	s_managerTable->set_manager(ENGINE_MANAGER_GRAPHIC_DEVICE, new GSharedPtr<IGVulkanDevice>(dev));
	s_managerTable->set_manager(ENGINE_MANAGER_WINDOW, new GSharedPtr<Window>(m_window));
	s_managerTable->set_manager(ENGINE_MANAGER_LOGGER, new GSharedPtr<IGLoggerManager>(s_logger));
	s_managerTable->set_manager(ENGINE_MANAGER_RESOURCE, new GSharedPtr<IGResourceManager>(s_resourceManager));
	s_managerTable->set_manager(ENGINE_MANAGER_JOB, new GSharedPtr<IJobManager>(s_jobManager));
	s_managerTable->set_manager(ENGINE_MANAGER_SHADER, new GSharedPtr<IGShaderManager>(new GShaderManager()));

	s_logger->enable_file_logging("logs/log_err.txt",LOG_LEVEL_ERROR);
	s_device = dev;
	GLoggerManager::set_instance(s_logger);
}

void GEngine::run()
{
	GSharedPtr<IGVulkanDevice>* graphicDevice = (GSharedPtr<IGVulkanDevice>*)m_managerTable->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);
#ifdef _DEBUG
	assert(m_inited == true);
#endif
	//X Run Phase
	m_window->show();	

	while (!get_exited()) {
		//X Tick Counter
		//X CPU GPU PERFORMANCE TRACKERS
		//X OnTick BeforeTick maybe
		//X Before tick pump messages and check global vals
		m_window->pump_messages();
		
		//X Check game wanted to exit
		if (m_window->wants_close())
		{
			request_exit();
		}
		if (m_vulkanSwapchain->need_handle())
		{
			s_device->get_queue_fence()->wait();

			m_vulkanSwapchain->handle();
		}
		tick();
	}
	
	s_device->get_queue_fence()->wait();

	exit();
}


void GEngine::exit()
{
	s_logger->log_d("GEngine", "Beginning to destroy application implementation");

	m_impl->destroy();
	//X Before Uninitialize Game

	//X Destroy global managers
	//X Destroy inner things
	s_logger->log_d("GEngine", "Beginning to destroy main viewport");

	m_vulkanSwapchain->destroy();

	delete m_vulkanSwapchain;


	//X Destroys main surface
	vkDestroySurfaceKHR((VkInstance)m_vulkanApp->get_vk_instance(), m_mainSurface, nullptr);


	s_logger->log_d("GEngine", "Beginning to destroy resource manager");

	s_resourceManager->destroy();

	s_logger->log_d("GEngine", "Beginning to destroy graphic device");

	s_device->destroy();

	s_logger->log_d("GEngine", "Beginning to destroy vulkan device");


	m_vulkanApp->destroy();


	s_logger->log_d("GEngine", "Beginning to destroy global manager containers");

	// X Delete table
	((ManagerTable*)m_managerTable)->delete_managers();
	delete m_managerTable;
}

void GEngine::tick()
{

	//X BeforeUpdate
	//X Update
	//X After Update

	if (m_impl->before_update())
	{
		m_impl->update();

		m_impl->after_update();
		//X Physic Collisions


		//X Before Draw
		//X Draw
		//X After Draw

		if (before_render() && m_impl->before_render())
		{
			m_impl->render();
			after_render();
			m_impl->after_render();
		}

		//X DoSmth and AfterDoSmth are inside the brackets of BeforeDoSmth  and Befores returns bool and check with if the return bool value 
	}
}

bool GEngine::before_render()
{
	s_device->get_queue_fence()->wait();
	return m_vulkanSwapchain->acquire_draw_image(s_device->get_image_acquired_semaphore()) && s_device->prepare_for_rendering();
	
}

void GEngine::after_render()
{
	VkCommandBuffer buff = s_device->get_main_command_buffer()->get_handle();

	VkSemaphore semaphores = s_device->get_render_complete_semaphore()->get_semaphore();
	VkSemaphore waitSemaphore = s_device->get_image_acquired_semaphore()->get_semaphore();
	auto waitSemaphores = s_device->get_wait_semaphores();
	
	std::vector<VkSemaphore> wSemaphores(waitSemaphores->size() + 1);
	
	wSemaphores[0] = waitSemaphore;
	
	for (int i = 1; i < (*waitSemaphores).size()+1; i++)
	{
		wSemaphores[i] = (*waitSemaphores)[i - 1].first->get_semaphore();
	}

	VkSubmitInfo inf = {};
	
	std::vector< VkPipelineStageFlags> waitStages(waitSemaphores->size() + 1);
	waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	for (int i = 1; i < (*waitSemaphores).size() + 1; i++)
	{
		waitStages[i] = (*waitSemaphores)[i - 1].second;
	}
	
	inf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	inf.pNext = nullptr;
	inf.commandBufferCount = 1;
	inf.pCommandBuffers = &buff;
	inf.signalSemaphoreCount = 1;
	inf.pSignalSemaphores = &semaphores;
	inf.pWaitDstStageMask = waitStages.data();
	inf.waitSemaphoreCount = wSemaphores.size();
	inf.pWaitSemaphores = wSemaphores.data();

	vkQueueSubmit(s_device->as_logical_device()->get_render_queue()->get_queue(), 1, &inf, s_device->get_queue_fence()->get_fence());

	m_vulkanSwapchain->present_image(1,s_device->get_render_complete_semaphore());
}

//IGVulkanMainViewport* GEngine::create_viewport(int width, int height)
//{
//	return new GVulkanMainViewport();
//}
void GEngine::init(GApplicationImpl* impl)
{

	m_impl = impl;

	bool inited = s_logger->init();
	
	if (!inited)
		return;

	s_logger->log_d("GEngine", "Beginning to initialize GLFW");

	auto resglfw = glfwInit();

	if (resglfw != GLFW_TRUE)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing GLFW. Engine shutdown");
		return;
	}

	GInjectManagerHelper help(s_managerTable);

	help.add_manager_spec(ENGINE_MANAGER_WINDOW,(void*)&m_window->get_window_props());

	m_impl->inject_managers(&help);

	GSharedPtr<IGVulkanDevice>* graphicDevice = (GSharedPtr<IGVulkanDevice>*)m_managerTable->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);
	GSharedPtr<IGShaderManager>* shaderManager = (GSharedPtr<IGShaderManager>*)s_managerTable->get_engine_manager_managed(ENGINE_MANAGER_SHADER);


	s_logger->log_d("GEngine","Beginning to initialize window");

	uint32_t initedI = m_window->init();
	
	if (initedI != 0)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing window. Engine shutdown");
		return;
	}

	s_logger->log_d("GEngine", "Beginning to initialize vulkan app");

	inited = m_vulkanApp->init();

	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing vulkan app. Engine shutdown");
		return;
	}

	s_logger->log_d("GEngine", "Beginning to initialize graphic device");

	inited = (*graphicDevice)->init();

	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing graphic device. Engine shutdown");
		return;
	}

	s_logger->log_d("GEngine", "Beginning to initialize shader manager");

	inited = shaderManager->get()->init();

	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing shader manager. Engine shutdown");
		return;
	}

	s_logger->log_d("GEngine", "Beginning to initialize resource manager");

	inited = s_resourceManager->init();
	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing resource manager. Engine shutdown");
		return;
	}

	//X IT WORKS FOR ONLY GLFW WINDOW
	VkResult res = glfwCreateWindowSurface((VkInstance)m_vulkanApp->get_vk_instance(), (GLFWwindow*)m_window->get_native_handler(), nullptr,&m_mainSurface);
	VkSurfaceFormatKHR surfaceFormat = {};
	surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

	s_logger->log_d("GEngine", "Beginning to initialize main viewport");

	m_vulkanSwapchain = m_vulkanSwapchain = new GVulkanSwapchain((GVulkanLogicalDevice*)s_device->as_logical_device().get(), m_mainSurface, SwapchainImageCount, m_window->get_window_props().width, m_window->get_window_props().height, surfaceFormat, (*graphicDevice)->as_logical_device()->get_present_queue());

	inited = m_vulkanSwapchain->init();

	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing swapchain. Engine shutdown");
		return;
	}


#ifdef _DEBUG
	m_inited = true;
#endif

	s_logger->log_d("GEngine", "Beginning to initialize application implementation");
	inited = m_impl->init(this);

	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing application implementation. Engine shutdown");
		return;
	}
}

Window* GEngine::get_main_window()
{
#ifdef _DEBUG
	assert(m_inited == true);
#endif
	return m_window;
}

IManagerTable* GEngine::get_manager_table()
{
	return m_managerTable;
}

IGVulkanApp* GEngine::get_app()
{
	return m_vulkanApp.get();
}

IGVulkanViewport* GEngine::create_offscreen_viewport(IGVulkanDescriptorCreator* descriptor)
{
	return new GVulkanOffScreenViewport(s_device->as_logical_device().get(), descriptor);
}

IGVulkanViewport* GEngine::get_viewport()
{
	return m_vulkanSwapchain->get_viewport();
}

void GEngine::destroy_offscreen_viewport(IGVulkanViewport* port)
{
	delete port;
}

GEngine* GEngine::get_instance()
{
	return s_engine;
}

IGVulkanSwapchain* GEngine::get_swapchain()
{
	return m_vulkanSwapchain;
}

ENGINE_API GEngine* create_the_engine()
{
	s_engine = new GEngine();
	return s_engine;
}