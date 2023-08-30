#include <volk/volk.h>
#define VK_NO_PROTOTYPES
#include "internal/imgui_layer.h"
#include <imgui/imgui.h>

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>


#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/ivulkan_app.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "internal/window/gimgui_viewport_window.h"

#include "public/platform/window.h"
#include "internal/rendering/vulkan/gscene_renderer.h"
#include "internal/imgui_window_manager.h"
#include "internal/window/gimgui_texteditor_window.h"
#include "internal/window/gimgui_content_browser_window.h"
#include "internal/window/gimgui_log_window.h"

#include "internal/menu/gtheme_menu.h"

ImGuiLayer::ImGuiLayer(IGVulkanViewport* viewport,Window* window, IGVulkanApp* app, IGVulkanDevice* dev)
{
	m_viewport = viewport;
	m_window = window;
	m_dev = dev;
	m_app = app;
	m_descriptorPool = nullptr;
	//X TODO : GDNEWDA
	m_windowManager = new ImGuiWindowManager();
	m_renderViewportWindow = new GImGuiViewportWindow();
	m_contentBrowserWindow = new GImGuiContentBrowserWindow();
	m_logWindow = new GImGuiLogWindow();

	m_sceneRenderer = new GSceneRenderer(viewport,dev);

}

ImGuiLayer::~ImGuiLayer()
{
}

bool ImGuiLayer::init()
{


	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	auto err = vkCreateDescriptorPool((VkDevice)m_dev->as_logical_device()->get_vk_device(), &pool_info, nullptr, &m_descriptorPool);

	VkInstance ins = (VkInstance)m_app->get_vk_instance();
	ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
		return vkGetInstanceProcAddr((reinterpret_cast<VkInstance>(vulkan_instance)), function_name);
		}, ins);




	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//ImGui::StyleColorsDark();
	//auto& style = ImGui::GetStyle();
	////!: Styles
	//style.ScrollbarSize = 8.f;
	//style.WindowRounding = 3.f;
	////!: Colors
	//auto colors = style.Colors;
	//colors[ImGuiCol_::ImGuiCol_WindowBg] = ImVec4(38.f / 255.f, 43.f / 255.f, 58.f / 255.f, 1.f);
	//colors[ImGuiCol_::ImGuiCol_TitleBg] = ImVec4(51.f / 255.f, 57.f / 255.f, 79.f / 255.f, 1.f);
	//colors[ImGuiCol_::ImGuiCol_TitleBgActive] = ImVec4(51.f / 255.f, 57.f / 255.f, 79.f / 255.f, 1.f);
	//colors[ImGuiCol_::ImGuiCol_MenuBarBg] = ImVec4(32.f / 255.f, 36.f / 255.f, 48.f / 255.f, 1.f);
	//colors[ImGuiCol_::ImGuiCol_CheckMark] = ImVec4(1.f, 1.f, 1.f, 1.f);
	//colors[ImGuiCol_::ImGuiCol_Border] = ImVec4(1.f, 1.f, 1.f, 1.f);

	///*colors[ImGuiCol_::ImGuiCol_FrameBg] = ImVec4(1.f, 1.f, 1.f, 0.f);
	//colors[ImGuiCol_::ImGuiCol_FrameBgHovered] = ImVec4(1.f, 1.f, 1.f, 0.f);
	//colors[ImGuiCol_::ImGuiCol_FrameBgActive] = ImVec4(1.f, 1.f, 1.f, 0.f);*/


	//style.WindowBorderSize = 2;
	//style.WindowRounding = 0;
	//style.WindowMenuButtonPosition = ImGuiDir_::ImGuiDir_None;	
	auto r = ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)m_window->get_native_handler(), true);

	IGVulkanQueue* queue = m_dev->as_logical_device()->get_render_queue();
	

	//X TODO : GET IMGAES FROM SWAPCHAIN
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = ins;
	init_info.PhysicalDevice =(VkPhysicalDevice)m_dev->as_physical_device()->get_vk_physical_device();
	init_info.Device = (VkDevice)m_dev->as_logical_device()->get_vk_device();
	init_info.QueueFamily = queue->get_queue_index();
	init_info.Queue = queue->get_queue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_descriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;

	ImGui_ImplVulkan_Init(&init_info, (VkRenderPass)(m_viewport->get_vk_current_image_renderpass()));
	
	GVulkanCommandBuffer* cmd = m_dev->get_single_time_command_buffer();
	
	ImGui_ImplVulkan_CreateFontsTexture(cmd->get_handle());

	m_dev->execute_single_time_command_buffer_and_wait();

	ImGui_ImplVulkan_DestroyFontUploadObjects();
	//vkDestroyDescriptorPool(device->m_device, imguiPool, nullptr);



	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	//X TODO : BACKTRACE DELETE

	bool inited = m_windowManager->init();

	if (!inited)
		return false;

	m_windowManager->create_imgui_menu(new GThemeMenu());
	
	inited = m_sceneRenderer->init();
	
	if (!inited)
		return false;


	bool created = m_windowManager->create_imgui_window(m_renderViewportWindow, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete m_renderViewportWindow;
		m_renderViewportWindow = nullptr;
	}
	created = m_windowManager->create_imgui_window(m_contentBrowserWindow, GIMGUIWINDOWDIR_BOTTOM);
	if (!created)
	{
		delete m_contentBrowserWindow;
		m_contentBrowserWindow = nullptr;
	}
	created = m_windowManager->create_imgui_window(m_logWindow, GIMGUIWINDOWDIR_BOTTOM);
	if (!created)
	{
		delete m_logWindow;
		m_logWindow = nullptr;
	}
	return true;
}

void ImGuiLayer::destroy()
{
	m_sceneRenderer->destroy();
	delete m_sceneRenderer;

	m_windowManager->destroy();
	//X TODO : Custom Deleter
	delete m_windowManager;
	//vkDestroyDescriptorPool(dev, imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool((VkDevice)m_dev->as_logical_device()->get_vk_device(), m_descriptorPool, nullptr);

}

bool ImGuiLayer::before_render()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	return true;
}

void ImGuiLayer::render(GVulkanCommandBuffer* cmd)
{


	m_windowManager->render_windows();

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->get_handle());


	m_sceneRenderer->render_the_scene();

	//X After drawing windows draw the viewport

}

void ImGuiLayer::set_viewport(IGVulkanViewport* viewport)
{
	m_renderViewportWindow->set_the_viewport(viewport);
	m_sceneRenderer->set_the_viewport(viewport);
}

ImGuiWindowManager* ImGuiLayer::get_window_manager()
{
	return m_windowManager;
}

GImGuiLogWindow* ImGuiLayer::get_log_window()
{
	return m_logWindow;
}
