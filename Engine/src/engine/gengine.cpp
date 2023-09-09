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
#include "internal/engine/rendering/vulkan/gvulkan_offscreen_depth_viewport.h"
#include "internal/engine/rendering/gvulkan_frame_data.h"
#include "internal/engine/rendering/vulkan/gvulkan_chained_viewport.h"
#include "internal/engine/manager/gcamera_manager.h"
#include "internal/engine/manager/gtimer_manager.h"
#include "internal/engine/manager/gpipeline_object_manager.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static uint32_t SwapchainImageCount = 3;
static uint32_t FRAME_IN_FLIGHT = 2;

static GVulkanDevice* s_device;
static GLoggerManager* s_logger;
static GResourceManager* s_resourceManager;
static GEngine* s_engine;
static GJobManager* s_jobManager;
static ManagerTable* s_managerTable;
static IGCameraManager* s_cameraManager;
static GTimerManager* s_timer;
static IGPipelineObjectManager* s_pipelineManager;
GEngine::GEngine()
{
	m_window = create_default_window();
	m_impl = nullptr;

	//X TODO : USE GDNEWD
	s_timer = new GTimerManager();
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
	s_managerTable->set_manager(ENGINE_MANAGER_CAMERA, new GSharedPtr<IGCameraManager>(new GCameraManager(FRAME_IN_FLIGHT)));

	s_logger->enable_file_logging("logs/log_err.txt",LOG_LEVEL_ERROR);
	s_device = dev;
	GLoggerManager::set_instance(s_logger);

	m_currentFrame = 0;
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
		auto deltaTime = s_timer->calculate_delta_time();
		int fps = 0;
		bool calculatedNewFps = s_timer->calc_fps(fps);

		//X OnTick BeforeTick maybe
		//X Before tick pump messages and check global vals
		m_window->pump_messages();
		
		//X Check game wanted to exit
		if (m_window->wants_close())
		{
			request_exit();
		}
		if (m_vulkanSwapchain->need_handle() || !m_recreationQueues.empty())
		{
			wait_all_frame_data();
			if(m_vulkanSwapchain->need_handle())
				m_vulkanSwapchain->handle();

			while (!m_recreationQueues.empty())
			{
				auto fun = m_recreationQueues.front();
				m_recreationQueues.pop();
				fun();
			}
		}
		tick(deltaTime);
	}
	
	wait_all_frame_data();

	exit();
}

void GEngine::wait_all_frame_data()
{
	for (int i = 0; i < m_frames.size(); i++)
	{
		m_frames[i]->wait_fence();
	}
}

void GEngine::exit()
{
	s_logger->log_d("GEngine", "Beginning to destroy application implementation");

	m_impl->destroy();
	//X Before Uninitialize Game
	for (int i = 0; i < m_frames.size(); i++)
	{
		m_frames[i]->destroy();
		delete m_frames[i];
	}
	//X Destroy global managers
	//X Destroy inner things
	s_logger->log_d("GEngine", "Beginning to destroy main viewport");
	
	s_cameraManager->destroy();

	s_pipelineManager->destroy();

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


	//X TODO WILL BE DELETED
	delete s_timer;

	// X Delete table
	((ManagerTable*)m_managerTable)->delete_managers();
	delete m_managerTable;

}

void GEngine::tick(double deltaTime)
{

	//X BeforeUpdate
	//X Update
	//X After Update

	if (m_impl->before_update())
	{
		s_cameraManager->update(deltaTime);

		m_impl->update();

		m_impl->after_update();
		//X Physic Collisions


		//X Before Draw
		//X Draw
		//X After Draw

		if (before_render() && m_impl->before_render())
		{
			s_cameraManager->render(m_currentFrame);
			m_impl->render();
			after_render();
			m_impl->after_render();
		}

		//X DoSmth and AfterDoSmth are inside the brackets of BeforeDoSmth  and Befores returns bool and check with if the return bool value 
	}
}

void GEngine::add_recreation(std::function<void()> recreationFun)
{
	m_recreationQueues.emplace(recreationFun);
}
bool GEngine::before_render()
{

	m_frames[m_currentFrame]->wait_fence();
	return m_vulkanSwapchain->acquire_draw_image(m_frames[m_currentFrame]->get_image_acquired_semaphore()) && m_frames[m_currentFrame]->prepare_for_rendering();
	
}

void GEngine::after_render()
{
	m_frames[m_currentFrame]->submit_the_cmd();

	//X Wait here to present the last image
	m_vulkanSwapchain->present_image(1, m_frames[m_currentFrame]->get_render_finished_semaphore());
	m_currentFrame = (m_currentFrame + 1) % FRAME_IN_FLIGHT;
}
uint32_t GEngine::get_current_frame()
{
	return m_currentFrame;
}

uint32_t GEngine::get_frame_count()
{
	return m_frames.size();
}
IGVulkanFrameData* GEngine::get_frame_data_by_index(uint32_t index)
{
	return m_frames[m_currentFrame];
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
	GSharedPtr<IGCameraManager>* cameraManager = (GSharedPtr<IGCameraManager>*)s_managerTable->get_engine_manager_managed(ENGINE_MANAGER_CAMERA);
	s_cameraManager = cameraManager->get();

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

	auto pipelineManager = new GPipelineObjectManager(graphicDevice->get()->as_logical_device().get(), surfaceFormat.format, FRAME_IN_FLIGHT);
	s_managerTable->set_manager(ENGINE_MANAGER_PIPELINE_OBJECT, new GSharedPtr<IGPipelineObjectManager>(pipelineManager));
	s_pipelineManager = pipelineManager;

	inited = s_pipelineManager->init();
	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing Pipeline Manager. Engine shutdown");
		return;
	}
	
	inited = s_cameraManager->init();
	
	if (!inited)
	{
		s_logger->log_c("GEngine", "Unknown error occured while initializing camera. Engine shutdown");
		return;
	}

	m_frames.resize(FRAME_IN_FLIGHT);
	auto logicalDev = s_device->as_logical_device().get();
	for (int i = 0; i < m_frames.size(); i++)
	{
		m_frames[i] = new GVulkanFrameData(logicalDev,i, logicalDev->get_render_queue());
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
	assert(false);
	return new GVulkanOffScreenViewport(s_device->as_logical_device().get(), descriptor);
}

IGVulkanViewport* GEngine::create_offscreen_viewport_depth(IGVulkanDescriptorCreator* descriptor)
{
	return new GVulkanOffScreenDepthViewport(((GSharedPtr<IGPipelineObjectManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_PIPELINE_OBJECT))->get(),s_device->as_logical_device().get(), descriptor);
}

IGVulkanChainedViewport* GEngine::create_offscreen_viewport_depth_chained(IGVulkanDescriptorCreator* descriptor, uint32_t imageCount)
{
	assert(false);
	return new GVulkanChainedViewport(s_device->as_logical_device().get(), descriptor,imageCount);
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