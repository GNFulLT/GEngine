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
#include "engine/manager/iglogger_manager.h"
#include "engine/io/iowning_glogger.h"
#include "gobject/gobject.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "engine/manager/iinject_manager_helper.h"
#include "public/platform/window_props.h"
#include "internal/window/gimgui_log_window.h"
#include "internal/manager/geditor_shader_manager.h"
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include "engine/rendering/vulkan/ivulkan_swapchain.h"
#include "engine/manager/igresource_manager.h"
#include "engine/resource/igshader_resource.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"
#include "engine/rendering/igvulkan_frame_data.h"
#include "engine/rendering/vulkan/igvulkan_chained_viewport.h"
#include "internal/rendering/gfps_camera_positioner.h"
#include "engine/imanager_table.h"
#include "engine/manager/igcamera_manager.h"

IGVulkanLogicalDevice* s_device;

inline VkDescriptorSetLayoutBinding descriptor_set_layout_binding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags, uint32_t descriptorCount = 1)
{
    return VkDescriptorSetLayoutBinding{
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = descriptorCount,
        .stageFlags = flags,
        .pImmutableSamplers = nullptr
    };
}

GEditorShaderManager* s_shaderManager;
EditorApplicationImpl* EditorApplicationImpl::get_instance()
{
    return s_instance;
}

void EditorApplicationImpl::destroy()
{
    m_renderViewport->destroy(false);
    
    m_engine->destroy_offscreen_viewport(m_renderViewport);
    
    m_imguiLayer->destroy();

    delete m_imguiDescriptorCreator;
}

void EditorApplicationImpl::inject_managers(IInjectManagerHelper* helper)
{
    auto prop = (WindowProps*)helper->get_manager_spec(ENGINE_MANAGER_WINDOW);
    if(prop != nullptr)
        prop->hasCaption = false;

    s_shaderManager = new GEditorShaderManager();
    auto defaultMng = helper->swap_and_get_managed(ENGINE_MANAGER_SHADER, s_shaderManager);
    s_shaderManager->set_default((GSharedPtr<IGShaderManager>*)defaultMng);
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
    auto currIndex = m_engine->get_current_frame();
    //m_renderViewport->set_image_index(currIndex);
    GVulkanCommandBuffer* cmd = m_engine->get_frame_data_by_index(currIndex)->get_the_main_cmd();
    mainViewport->begin_draw_cmd(cmd);

    //X TODO : Layered Architecture here

    m_imguiLayer->render(cmd);


    // Renderpass
    mainViewport->end_draw_cmd(cmd);   
}

void EditorApplicationImpl::after_render()
{
}

bool EditorApplicationImpl::init(GEngine* engine)
{
    s_instance = this;
    
    m_engine = engine;
    IManagerTable* table = m_engine->get_manager_table();
    auto logger = (GSharedPtr<IGLoggerManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_LOGGER);
    auto resourceManager = ((GSharedPtr<IGResourceManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE))->get();

    m_logger = (*logger)->create_owning_glogger("EditorLayer");
    m_logWindwLogger = (*logger)->create_owning_glogger("Editor",false);
    s_shaderManager->editor_init();
    auto dev = (GSharedPtr<IGVulkanDevice>*)table->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);
    s_device = dev->get()->as_logical_device().get();
    m_imguiDescriptorCreator = new GImGuiDescriptorCreator(dev->get()->as_logical_device().get());


    m_imguiLayer = new ImGuiLayer(engine->get_viewport(),engine->get_main_window(),engine->get_app(),dev->get());
    
    m_logger->log_d("Initializing Global Pointers");
    glfwInit();
    VkResult res= volkInitialize();
    volkLoadInstance((VkInstance)m_engine->get_app()->get_vk_instance());
    volkLoadDevice((VkDevice)dev->get()->as_logical_device()->get_vk_device());
    
    m_logger->log_d("Initializing ImGuiLayer");

    m_imguiLayer->init();


    m_logger->log_d("Editor Initializing Finished");


    m_logger->log_d("Trying to get type glogger");

    GType type = GTypeUtils::get_type_from_name("GLoggerManager");
    
    if (type.is_valid())
    {
        m_logger->log_d("Got gtype: Type name is : ");

        m_logger->log_d(type.get_name().data());

        GFunction func = type.get_function_by_name("log_d");
        if (func.is_valid())
        {
            m_logger->log_d("Got function: Function name is : ");
            m_logger->log_d(func.get_name().data());
        }
    }
    else
    {
        m_logger->log_d("Couldn't get GLoggerManager Type");

    }

    m_logger->log_d("Creating ImGui Descriptor Pool Wrapper");

    //X First create the descriptor pool wrapper for imgui


    //X Create the game viewport here

    m_logger->log_d("Creating Render Viewport");
    
    m_renderViewport = engine->create_offscreen_viewport_depth(m_imguiDescriptorCreator);

    m_imguiLayer->set_viewport(m_renderViewport);

    
    if (auto logWin = m_imguiLayer->get_log_window();logWin != nullptr)
    {
        m_logWindwLogger->add_sink(logWin->create_sink());
    }

    m_fpsCameraPositioner.reset(new GEditorFPSCameraPositioner(m_imguiLayer->get_window_manager()));

    ((GSharedPtr<IGCameraManager>*)engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get()->set_positioner(m_fpsCameraPositioner.get());
  
   return true;    
}

GSharedPtr<IOwningGLogger> EditorApplicationImpl::get_editor_logger()
{
    return m_logger;
}

GSharedPtr<IOwningGLogger> EditorApplicationImpl::get_editor_log_window_logger()
{
    return m_logWindwLogger;
}

GImGuiDescriptorCreator* EditorApplicationImpl::get_descriptor_creator()
{
    return m_imguiDescriptorCreator;
}

ImGuiLayer* EditorApplicationImpl::get_editor_layer()
{
    return m_imguiLayer;
}

EDITOR_API GApplicationImpl* create_the_editor()
{
    return new EditorApplicationImpl();
}