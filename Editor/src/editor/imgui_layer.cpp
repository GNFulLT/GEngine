#define VK_NO_PROTOTYPES
#include <volk/volk.h>
#include "internal/imgui_layer.h"
#include <imgui/imgui.h>

#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/ivulkan_app.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "internal/window/gimgui_viewport_window.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "editor/editor_application_impl.h"
#include "public/platform/window.h"
#include "internal/rendering/vulkan/gscene_renderer.h"
#include "internal/imgui_window_manager.h"
#include "internal/window/gimgui_texteditor_window.h"
#include "internal/window/gimgui_content_browser_window.h"
#include "internal/window/gimgui_log_window.h"
#include "internal/menu/gtheme_menu.h"
#include "internal/window/gimgui_properties_window.h"
#include "internal/window/gimgui_material_window.h"
#include "internal/window/gimgui_scene_window.h"
#include "internal/window/gimgui_albedo_port.h"
#include "internal/window/gimgui_position_port.h"
#include "internal/window/gimgui_normal_port.h"
#include "internal/window/gimgui_composition_window.h"
#include "internal/window/gimgui_pbr_window.h"
#include "internal/window/gimgui_sunshadow_window.h"
#include "internal/menu/gproject_menu.h"
#include "internal/menu/gwindow_menu.h"
#include "internal/window/provider/gimgui_script_window_provider.h"

#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/manager/igscene_manager.h"
#include "engine/rendering/renderer/igvulkan_deferred_renderer.h"

ImGuiLayer::ImGuiLayer(IGVulkanViewport* viewport,Window* window, IGVulkanApp* app, IGVulkanDevice* dev)
{
	m_sceneRenderer = nullptr;
	m_viewport = viewport;
	m_window = window;
	m_dev = dev;
	m_app = app;
	m_descriptorPool = nullptr;
	//X TODO : GDNEWDA
	m_windowManager = new ImGuiWindowManager();
	//m_renderViewportWindow = new GImGuiViewportWindow();
	m_contentBrowserWindow = new GImGuiContentBrowserWindow();
	m_logWindow = new GImGuiLogWindow();

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
	ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, [](const char* function_name, void* vulkan_instance) {
		return vkGetInstanceProcAddr((reinterpret_cast<VkInstance>(vulkan_instance)), function_name);
		}, ins);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
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
	init_info.MinImageCount = 2;
	init_info.ImageCount = 3;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;

	init_info.PipelineInfoMain.RenderPass = (VkRenderPass)(m_viewport->get_vk_current_image_renderpass());
	init_info.PipelineInfoMain.Subpass = 0;
	init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info);
	
	GVulkanCommandBuffer* cmd = m_dev->get_single_time_command_buffer();
	
	//ImGui_ImplVulkan_CreateFontsTexture(cmd->get_handle());

	m_dev->execute_single_time_command_buffer_and_wait();

	//ImGui_ImplVulkan_DestroyFontUploadObjects();
	//vkDestroyDescriptorPool(device->m_device, imguiPool, nullptr);



	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	//X TODO : BACKTRACE DELETE

	bool inited = m_windowManager->init();

	if (!inited)
		return false;
	m_windowManager->create_imgui_menu(new GProjectMenu());
	auto windowMenu = new GWindowMenu(m_windowManager);
	m_windowManager->create_imgui_menu(windowMenu);
	windowMenu->register_provider(new GImGuiScriptWindowProvider());
	m_windowManager->create_imgui_menu(new GThemeMenu());

	m_scene = new GImGuiSceneWindow();
	bool created = m_windowManager->create_imgui_window(m_scene, GIMGUIWINDOWDIR_LEFT);
	if (!created)
	{
		delete m_scene;
	}

	/*created = m_windowManager->create_imgui_window(m_renderViewportWindow, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete m_renderViewportWindow;
		m_renderViewportWindow = nullptr;
	}*/

	
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

	GImGuiPropertiesWindow* prp = new GImGuiPropertiesWindow();
	created = m_windowManager->create_imgui_window(prp, GIMGUIWINDOWDIR_RIGHT);
	if (!created)
	{
		delete prp;
	}

	GImGuiMaterialsWindow* material = new GImGuiMaterialsWindow();
	created = m_windowManager->create_imgui_window(material, GIMGUIWINDOWDIR_RIGHT_BOTTOM);
	if (!created)
	{
		delete material;
	}

	GImGuiPositionPortWindow* pos = new GImGuiPositionPortWindow();
	created = m_windowManager->create_imgui_window(pos, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete pos;
	}

	GImGuiNormalPortWindow* normal = new GImGuiNormalPortWindow();
	created = m_windowManager->create_imgui_window(normal, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete normal;
	}

	GImGuiAlbedoPortWindow* albedo = new GImGuiAlbedoPortWindow();
	created = m_windowManager->create_imgui_window(albedo, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete albedo;
	}
	GImGuiPBRPortWindow* pbr = new GImGuiPBRPortWindow();
	created = m_windowManager->create_imgui_window(pbr, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete pbr;
	}

	GImGuiSunShadowPortWindow* sunShadow = new GImGuiSunShadowPortWindow();
	created = m_windowManager->create_imgui_window(sunShadow, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete sunShadow;
	}

	GImGuiCompositionPortWindow* comp = new GImGuiCompositionPortWindow();
	created = m_windowManager->create_imgui_window(comp, GIMGUIWINDOWDIR_MIDDLE);
	if (!created)
	{
		delete comp;
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

void ImGuiLayer::set_viewport(IGVulkanNamedDeferredViewport* viewport, IGSceneManager* sceneManager)
{
	auto deferredRenderer = sceneManager->get_deferred_renderer();

	auto deferredVp = viewport;
	deferredVp->init(640, 320);

	EditorApplicationImpl::get_instance()->positionPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_position_attachment(),
		deferredVp->get_sampler_for_named_attachment("position_attachment")).value();
	EditorApplicationImpl::get_instance()->normalPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_emission_attachment(),
		deferredVp->get_sampler_for_named_attachment("normal_attachment")).value();
	EditorApplicationImpl::get_instance()->albedoPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_albedo_attachment(),
		deferredVp->get_sampler_for_named_attachment("albedo_attachment")).value();
	EditorApplicationImpl::get_instance()->compositionPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_composition_attachment(),
		deferredVp->get_sampler_for_named_attachment("composition_attachment")).value();
	EditorApplicationImpl::get_instance()->pbrPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_pbr_attachment(),
		deferredVp->get_sampler_for_named_attachment("composition_attachment")).value();

	sceneManager->init_deferred_renderer(deferredVp);
	
	//m_renderViewportWindow->set_the_viewport(viewport);
	m_sceneRenderer = new GSceneRenderer(viewport, m_dev);
	m_sceneRenderer->init();
	
	m_sceneRenderer->set_the_viewport(viewport);

	EditorApplicationImpl::get_instance()->sunShadowPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_depth_texture(deferredRenderer->get_sun_shadow_attachment(),
		deferredVp->get_sampler_for_named_attachment("composition_attachment")).value();
}

ImGuiWindowManager* ImGuiLayer::get_window_manager()
{
	return m_windowManager;
}

GImGuiSceneWindow* ImGuiLayer::get_scene_window()
{
	return m_scene;
}

GImGuiLogWindow* ImGuiLayer::get_log_window()
{
	return m_logWindow;
}
