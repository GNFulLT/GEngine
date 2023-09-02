#include "volk.h"

#include <expected>
#include "internal/rendering/vulkan/gscene_renderer.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/vulkan_memory.h"
#include "engine/resource/igshader_resource.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/manager/igresource_manager.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/manager/igshader_manager.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"

static int ct = 0;

GSceneRenderer::GSceneRenderer(IGVulkanViewport* viewport,IGVulkanDevice* device)
{
	m_viewport = viewport;
	m_device = device;
	m_cmd = nullptr;
	m_semaphore = nullptr;

	m_vkViewport.height = viewport->get_height();
	m_vkViewport.width = viewport->get_width();
	m_vkViewport.minDepth = 0;
	m_vkViewport.maxDepth = 1;
	m_vkViewport.x = 0;
	m_vkViewport.y = 0;

	m_vkScissor.extent = { viewport->get_width(),viewport->get_height() };
	m_vkScissor.offset = {0,0};
}

void GSceneRenderer::render_the_scene()
{
	m_device->add_wait_semaphore_for_this_frame(m_semaphore, (int)VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	
	auto vp = m_viewport;
	vp->begin_draw_cmd(m_cmd);

	vkCmdBindPipeline(m_cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicPipeline->get_pipeline());

	if (m_vkViewport.height != m_viewport->get_height() || m_vkViewport.width != m_viewport->get_width())
	{
		m_vkViewport.height = m_viewport->get_height();
		m_vkViewport.width = m_viewport->get_width();
		m_vkScissor.extent.width = m_vkViewport.width;
		m_vkScissor.extent.height = m_vkViewport.height;
	}

	vkCmdSetViewport(m_cmd->get_handle(), 0, 1, &m_vkViewport);
	vkCmdSetScissor(m_cmd->get_handle(), 0, 1, &m_vkScissor);
	
	vkCmdDraw(m_cmd->get_handle(), 3, 1, 0, 0);

	vp->end_draw_cmd(m_cmd);

	auto cmd = m_cmd->get_handle();
	auto smph = m_semaphore->get_semaphore();
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo inf = {};
	inf.pNext = nullptr;
	inf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	inf.commandBufferCount = 1;
	inf.pCommandBuffers = &cmd;
	inf.waitSemaphoreCount = 0;
	inf.signalSemaphoreCount = 1;
	inf.pSignalSemaphores = &smph;
	inf.pWaitDstStageMask = &waitStage;



	m_device->execute_cmd_from_main(m_cmd,&inf,nullptr);

}

bool GSceneRenderer::init()
{
	EditorApplicationImpl::get_instance()->get_editor_logger()->log_d("Create the main cmd");
	m_cmd = m_device->create_cmd_from_main_pool();

	m_semaphore = m_device->create_semaphore(false);

	auto table = EditorApplicationImpl::get_instance()->m_engine->get_manager_table();

	auto resourceManager = ((GSharedPtr<IGResourceManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE))->get();
	auto dev = (GSharedPtr<IGVulkanDevice>*)table->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE);
	auto s_shaderManager = ((GSharedPtr<IGShaderManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_SHADER))->get();


	auto shaderRes = resourceManager->create_shader_resource("BasicVertex", "EditorResource", "./basic_vertex.glsl_vert");
	if (shaderRes.has_value())
	{
		m_basicVertexShader = GSharedPtr<IGShaderResource>(shaderRes.value());
		auto loaded = m_basicVertexShader->load();
		int a = 5;
	}
	else
	{
		assert(false);
	}

	shaderRes = resourceManager->create_shader_resource("BasicFrag", "EditorResource", "./basic_frag.glsl_frag");
	if (shaderRes.has_value())
	{
		m_basicFragShader = GSharedPtr<IGShaderResource>(shaderRes.value());
		auto loaded = m_basicFragShader->load();
		int a = 5;
	}
	else
	{
		assert(false);
	}

	auto s_device = dev->get()->as_logical_device().get();
	auto stageRes = s_shaderManager->create_shader_stage_from_shader_res(m_basicVertexShader);
	if (stageRes.has_value())
	{
		m_vertexShaderStage = stageRes.value();
	}
	else
	{
		assert(false);
	}

	auto stageRes2 = s_shaderManager->create_shader_stage_from_shader_res(m_basicFragShader);
	if (stageRes2.has_value())
	{
		m_fragShaderStage = stageRes2.value();
	}
	else
	{
		assert(false);
	}

	std::vector<IGVulkanGraphicPipelineState*> m_states;

	m_states.push_back(s_device->create_vertex_input_state(nullptr, nullptr));
	m_states.push_back(s_device->create_default_input_assembly_state());
	m_states.push_back(s_device->create_default_rasterization_state());
	m_states.push_back(s_device->create_default_none_multisample_state());
	m_states.push_back(s_device->create_default_color_blend_state());
	m_states.push_back(s_device->create_default_viewport_state(1920, 1080));

	std::vector<IVulkanShaderStage*> shaderStages;
	shaderStages.push_back(m_vertexShaderStage);
	shaderStages.push_back(m_fragShaderStage);

	m_graphicPipeline = s_device->create_and_init_default_graphic_pipeline_for_vp(m_viewport, shaderStages, m_states);

	for (int i = 0; i < m_states.size(); i++)
	{
		auto state =  m_states[i];
		delete state;
	}



	return true;
}

void GSceneRenderer::destroy()
{
	if (m_graphicPipeline != nullptr)
	{
		m_graphicPipeline->destroy();
		delete m_graphicPipeline;
	}
	delete m_fragShaderStage;

	delete m_vertexShaderStage;

	if (m_basicFragShader.is_valid())
	{
		m_basicFragShader->destroy();
	}
	if (m_basicVertexShader.is_valid())
	{
		m_basicVertexShader->destroy();
	}

	m_device->destroy_semaphore(m_semaphore);
	m_device->destroy_cmd_main_pool(m_cmd);
	m_cmd = nullptr;
}

void GSceneRenderer::set_the_viewport(IGVulkanViewport* viewport)
{
	m_viewport = viewport;
	
}
