#include "engine/gengine.h"
#include "engine/globals.h"
#include "public/platform/window.h"

#include "internal/engine/rendering/vulkan/vulkan_device.h"
#include "internal/engine/rendering/vulkan/vulkan_app.h"
#include <cassert>
#include "internal/engine/manager_table.h"
#include "internal/engine/rendering/vulkan/vulkan_viewport.h"
#include "internal/engine/manager/glogger_manager.h"

#include "internal/engine/rendering/vulkan/vulkan_memory.h"
#include "internal/engine/rendering/vulkan/vulkan_queue.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include  <GLFW/glfw3.h>

static GVulkanDevice* s_device;
static GLoggerManager* s_logger;
GEngine::GEngine()
{
	m_window = create_default_window();
	m_impl = nullptr;

	//X TODO : USE GDNEWD
	m_vulkanApp = GSharedPtr<IGVulkanApp>(new GVulkanApp());
	auto managerTable = gdnew(ManagerTable);
	m_managerTable = managerTable;
	auto dev = new GVulkanDevice(m_vulkanApp);
	s_logger = new GLoggerManager();

	managerTable->set_manager(ENGINE_MANAGER_GRAPHIC_DEVICE, new GSharedPtr<IGVulkanDevice>(dev));
	managerTable->set_manager(ENGINE_MANAGER_WINDOW, new GSharedPtr<Window>(m_window));
	managerTable->set_manager(ENGINE_MANAGER_LOGGER, new GSharedPtr<IGLoggerManager>(s_logger));

	s_device = dev;
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
		if (m_mainViewport->need_handle())
		{
			s_device->get_queue_fence()->wait();

			m_mainViewport->handle();
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
	s_logger->log_d("GEngine", "Beginning to destroy main viewport");

	((GVulkanViewport*)m_mainViewport)->destroy();

	delete m_mainViewport;


	//X Destroys main surface
	vkDestroySurfaceKHR((VkInstance)m_vulkanApp->get_vk_instance(), m_mainSurface, nullptr);

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
	return m_mainViewport->acquire_draw_image(s_device->get_image_acquired_semaphore()) && s_device->prepare_for_rendering();
	
}

void GEngine::after_render()
{
	VkCommandBuffer buff = s_device->get_main_command_buffer()->get_handle();

	VkSemaphore semaphores = s_device->get_render_complete_semaphore()->get_semaphore();
	VkSemaphore waitSemaphore = s_device->get_image_acquired_semaphore()->get_semaphore();

	VkSubmitInfo inf = {};
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	inf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	inf.pNext = nullptr;
	inf.commandBufferCount = 1;
	inf.pCommandBuffers = &buff;
	inf.signalSemaphoreCount = 1;
	inf.pSignalSemaphores = &semaphores;
	inf.pWaitDstStageMask = &waitStage;
	inf.waitSemaphoreCount = 1;
	inf.pWaitSemaphores = &waitSemaphore;

	vkQueueSubmit(s_device->as_logical_device()->get_render_queue()->get_queue(), 1, &inf, s_device->get_queue_fence()->get_fence());

	m_mainViewport->present_image(1,s_device->get_render_complete_semaphore());
}

void GEngine::init(GApplicationImpl* impl)
{
	GSharedPtr<IGVulkanDevice>* graphicDevice = (GSharedPtr<IGVulkanDevice>*)m_managerTable->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);


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

	m_mainViewport = new GVulkanViewport((GVulkanLogicalDevice*)(*graphicDevice)->as_logical_device().get(), m_window->get_window_props().width, m_window->get_window_props().height);
	//X IT WORKS FOR ONLY GLFW WINDOW
	VkResult res = glfwCreateWindowSurface((VkInstance)m_vulkanApp->get_vk_instance(), (GLFWwindow*)m_window->get_native_handler(), nullptr,&m_mainSurface);
	VkSurfaceFormatKHR surfaceFormat = {};
	surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

	s_logger->log_d("GEngine", "Beginning to initialize main viewport");
	inited = ((GVulkanViewport*)m_mainViewport)->init(m_mainSurface, surfaceFormat,(*graphicDevice)->as_logical_device()->get_present_queue());

	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing main viewport. Engine shutdown");
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



IGVulkanViewport* GEngine::get_viewport()
{
	return m_mainViewport;
}

ENGINE_API GEngine* create_the_engine()
{
	return new GEngine();
}