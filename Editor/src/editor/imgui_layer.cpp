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

#include "public/platform/window.h"

#include "internal/imgui_window_manager.h"

ImGuiLayer::ImGuiLayer(IGVulkanViewport* viewport,Window* window, IGVulkanApp* app, IGVulkanDevice* dev)
{
	m_viewport = viewport;
	m_window = window;
	m_dev = dev;
	m_app = app;
	m_descriptorPool = nullptr;
	//X TODO : GDNEWDA
	m_windowManager = new ImGuiWindowManager();
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
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8.00f, 8.00f);
	style.FramePadding = ImVec2(5.00f, 2.00f);
	style.CellPadding = ImVec2(6.00f, 6.00f);
	style.ItemSpacing = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	style.IndentSpacing = 25;
	style.ScrollbarSize = 15;
	style.GrabMinSize = 10;
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;
	style.TabBorderSize = 1;
	style.WindowRounding = 7;
	style.ChildRounding = 4;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 3;
	style.LogSliderDeadzone = 4;
	style.TabRounding = 4;

	auto r = ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)m_window->get_native_handler(), true);

	IGVulkanQueue* queue = m_dev->as_logical_device()->get_render_queue();
	
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = ins;
	init_info.PhysicalDevice =(VkPhysicalDevice)m_dev->as_physical_device()->get_vk_physical_device();
	init_info.Device = (VkDevice)m_dev->as_logical_device()->get_vk_device();
	init_info.QueueFamily = queue->get_queue_index();
	init_info.Queue = queue->get_queue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_descriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = m_viewport->get_total_image() - 1;
	init_info.ImageCount = m_viewport->get_total_image();
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



	m_windowManager->init();

	return true;
}

void ImGuiLayer::destroy()
{
	m_windowManager->destroy();
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

}
