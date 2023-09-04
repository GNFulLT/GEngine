#include "volk.h"

#include <cstddef>
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
#include "engine/rendering/vulkan/igvulkan_vertex_buffer.h"
#include "internal/rendering/vulkan/gdefault_pipeline_injector.h"
#include "public/math/gcam.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "engine/rendering/igvulkan_frame_data.h"

static int ct = 0;

GSceneRenderer::GSceneRenderer(IGVulkanViewport* viewport,IGVulkanDevice* device)
{
	m_viewport = viewport;
	m_device = device;
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
	auto currIndex = EditorApplicationImpl::get_instance()->m_engine->get_current_frame();
	auto frameData = EditorApplicationImpl::get_instance()->m_engine->get_frame_data_by_index(currIndex);
	auto frameCmd = m_frameCmds[currIndex];

	frameData->add_wait_semaphore_for_this_frame(m_frameSemaphores[currIndex], (int)VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	frameCmd->reset();
	auto vp = m_viewport;
	vp->begin_draw_cmd(frameCmd);

	vkCmdBindPipeline(frameCmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicPipeline->get_pipeline());

	if (m_vkViewport.height != m_viewport->get_height() || m_vkViewport.width != m_viewport->get_width())
	{
		m_vkViewport.height = m_viewport->get_height();
		m_vkViewport.width = m_viewport->get_width();
		m_vkScissor.extent.width = m_vkViewport.width;
		m_vkScissor.extent.height = m_vkViewport.height;
	}

	vkCmdSetViewport(frameCmd->get_handle(), 0, 1, &m_vkViewport);
	vkCmdSetScissor(frameCmd->get_handle(), 0, 1, &m_vkScissor);
	
	VkDeviceSize offset = 0;
	VkBuffer buff = triangle->get_vertex_buffer()->get_vk_buffer();
	vkCmdBindVertexBuffers(frameCmd->get_handle(), 0, 1,&buff, &offset);
	
	auto gcamPos = gvec3(1.f, 0.f, -5.f);
	auto viewMatrix = translate(gcamPos);
	auto projMatrix = perspective(70.f, m_vkViewport.width / m_vkViewport.height, 0.1f, 1000.f);
	
	auto& modelMatrix = triangle->get_model_matrix();
	auto meshMatrix = projMatrix * viewMatrix * modelMatrix;

	//glm::vec3 camPos = { 0.f,0.f,-2.f };

	//glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	////camera projection
	//glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	//projection[1][1] *= -1;
	////model rotation
	//glm::mat4 model = glm::mat4(1.f);

	////calculate final mesh matrix
	//glm::mat4 mesh_matrix = projection * view * model;


	vkCmdPushConstants(frameCmd->get_handle(), m_graphicPipeline->get_pipeline_layout()->get_vk_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(gmat4), &meshMatrix);

	vkCmdDraw(frameCmd->get_handle(), 3, 1, 0, 0);

	vp->end_draw_cmd(frameCmd);

	auto cmd = frameCmd->get_handle();
	auto smph = m_frameSemaphores[currIndex]->get_semaphore();
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



	m_device->execute_cmd_from_main(frameCmd,&inf,nullptr);

}

bool GSceneRenderer::init()
{
	EditorApplicationImpl::get_instance()->get_editor_logger()->log_d("Create the main cmd");
	auto frameCount = EditorApplicationImpl::get_instance()->m_engine->get_frame_count();

	for (int i = 0; i < frameCount; i++)
	{
		auto fdata = EditorApplicationImpl::get_instance()->m_engine->get_frame_data_by_index(i);
		auto cmd = fdata->create_command_buffer_for_this_frame();
		cmd->init();
		m_frameCmds.push_back(cmd);
		m_frameSemaphores.push_back(m_device->create_semaphore(false));
	}


	

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
	
	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex_1);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector< VkVertexInputBindingDescription> bindingDescriptions;
	bindingDescriptions.push_back(mainBinding);

	std::vector<VkVertexInputAttributeDescription> inputAttributes;

	inputAttributes.resize(2);

	auto positionAttribute = &inputAttributes[0];
	positionAttribute->binding = 0;
	positionAttribute->location = 0;
	positionAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute->offset = offsetof(Vertex_1, position);

	auto colorAttribute = &inputAttributes[1];
	colorAttribute->binding = 0;
	colorAttribute->location = 1;
	colorAttribute->format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute->offset = offsetof(Vertex_1, color);

	


	m_states.push_back(s_device->create_vertex_input_state(&bindingDescriptions, &inputAttributes));
	m_states.push_back(s_device->create_default_input_assembly_state());
	m_states.push_back(s_device->create_default_rasterization_state());
	m_states.push_back(s_device->create_default_none_multisample_state());
	m_states.push_back(s_device->create_default_color_blend_state());
	m_states.push_back(s_device->create_default_viewport_state(1920, 1080));
	m_states.push_back(s_device->create_default_depth_stencil_state());

	std::vector<IVulkanShaderStage*> shaderStages;
	shaderStages.push_back(m_vertexShaderStage);
	shaderStages.push_back(m_fragShaderStage);

	m_graphicPipeline = s_device->create_and_init_graphic_pipeline_injector_for_vp(m_viewport, shaderStages, m_states,new GDefaultPipelineInjector(s_device));

	for (int i = 0; i < m_states.size(); i++)
	{
		auto state =  m_states[i];
		delete state;
	}
	std::vector<Vertex_1> pos;
	pos.resize(3);

	pos[0].position = { 1.f,1.f,0.f };
	pos[1].position = { -1.f, 1.f, 0.0f };
	pos[2].position = { 0.f,-1.f, 0.0f };

	pos[0].color = { 0.f,0.f,1.f };
	pos[1].color = { 0.f,0.f,1.f };
	pos[2].color = { 0.f,0.f,1.f };

	triangle = new Renderable(s_device,pos);

	triangle->init();
	return true;
}

void GSceneRenderer::destroy()
{
	triangle->destroy();
	delete triangle;
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

	for (int i = 0; i < m_frameCmds.size(); i++)
	{
		m_frameCmds[i]->destroy();
		m_device->destroy_semaphore(m_frameSemaphores[i]);
		delete m_frameCmds[i];
	}
	
}

void GSceneRenderer::set_the_viewport(IGVulkanViewport* viewport)
{
	m_viewport = viewport;
	
}
