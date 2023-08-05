#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "volk/volk.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/ivulkan_app.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "internal/imgui_layer.h"

void EditorApplicationImpl::destroy()
{
    m_imguiLayer->destroy();
}

bool EditorApplicationImpl::before_update()
{
    return true;
}

void EditorApplicationImpl::update()
{
}

void EditorApplicationImpl::after_update()
{
}

bool EditorApplicationImpl::before_render()
{
    m_imguiLayer->before_render();
    return true;
}

void EditorApplicationImpl::render()
{
    IGVulkanViewport* mainViewport = m_engine->get_viewport();
    IManagerTable* table = m_engine->get_manager_table();
    auto dev = (GSharedPtr<IGVulkanDevice>*)table->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);
    GVulkanCommandBuffer* cmd = dev->operator->()->get_main_command_buffer();
    dev->operator->()->as_logical_device()->begin_command_buffer_record(cmd);
    mainViewport->begin_draw_cmd(cmd);

    //X TODO : Layered Architecture here

    m_imguiLayer->render(cmd);


    // Renderpass
    mainViewport->end_draw_cmd(cmd);

    dev->operator->()->as_logical_device()->end_command_buffer_record(cmd);

   
}

void EditorApplicationImpl::after_render()
{
}

bool EditorApplicationImpl::init(GEngine* engine)
{
    m_engine = engine;
    IManagerTable* table = m_engine->get_manager_table();
    auto dev = (GSharedPtr<IGVulkanDevice>*)table->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);

    m_imguiLayer = new ImGuiLayer(engine->get_viewport(),engine->get_main_window(),engine->get_app(),dev->get());

    glfwInit();
    VkResult res= volkInitialize();
    volkLoadInstance((VkInstance)m_engine->get_app()->get_vk_instance());
    volkLoadDevice((VkDevice)dev->get()->as_logical_device()->get_vk_device());
    
    
    m_imguiLayer->init();

    return true;    
}

EDITOR_API GApplicationImpl* create_the_editor()
{
    return new EditorApplicationImpl();
}