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
#include "internal/rendering/vulkan/gcube_renderer.h"
#include "internal/rendering/vulkan/ggrid_renderer.h"
#include "internal/imgui_layer.h"
#include "internal/imgui_window_manager.h"
#include "internal/window/gimgui_grid_settings_window.h"
#include "internal/window/gimgui_scene_window.h"

static int perFrameCmd = 2;

static int ct = 0;
GSceneRenderer::GSceneRenderer(IGVulkanNamedDeferredViewport* viewport,IGVulkanDevice* device)
{
	m_viewport = viewport;
	m_device = device;
	m_vkViewport.height = viewport->get_viewport_area()->height;
	m_vkViewport.width = viewport->get_viewport_area()->width;
	m_vkViewport.minDepth = 0;
	m_vkViewport.maxDepth = 1;
	m_vkViewport.x = 0;
	m_vkViewport.y = 0;

	triangle = nullptr;
}

void GSceneRenderer::render_the_scene()
{

	auto currIndex = EditorApplicationImpl::get_instance()->m_engine->get_current_frame();
	auto frameData = EditorApplicationImpl::get_instance()->m_engine->get_frame_data_by_index(currIndex);
	auto cmdIndex = m_currentCmdIndex[currIndex];
	auto frameCmd = m_frameCmds[currIndex][cmdIndex];

	frameData->add_wait_semaphore_for_this_frame(m_frameSemaphores[currIndex], (int)VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	frameCmd->reset();

	auto vp = m_viewport;
	auto deferredRenderer = m_sceneManager->get_deferred_renderer();

	frameCmd->begin();
	deferredRenderer->fill_compute_cmd(frameCmd, currIndex);
	deferredRenderer->begin_and_end_fill_cmd_for_shadow(frameCmd, currIndex);
	vp->begin_draw_cmd(frameCmd);
	deferredRenderer->fill_deferred_cmd(frameCmd, currIndex);




	//vkCmdBindPipeline(frameCmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,m_graphicPipeline->get_pipeline());
	////X TODO : Layout Cache
	//m_graphicPipeline->bind_sets(frameCmd,currIndex);

	//vkCmdSetViewport(frameCmd->get_handle(), 0, 1, vp->get_viewport_area());
	//vkCmdSetScissor(frameCmd->get_handle(), 0, 1, vp->get_scissor_area());
	//
	//VkDeviceSize offset = 0;
	//VkBuffer buff = triangle->get_vertex_buffer()->get_vk_buffer();
	//vkCmdBindVertexBuffers(frameCmd->get_handle(), 0, 1,&buff, &offset);
	
	//auto& modelMatrix = triangle->get_model_matrix();

	//glm::vec3 camPos = { 0.f,0.f,-2.f };

	//glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	////camera projection
	//glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	//projection[1][1] *= -1;
	////model rotation
	//glm::mat4 model = glm::mat4(1.f);

	////calculate final mesh matrix
	//glm::mat4 mesh_matrix = projection * view * model;

	/*auto& modelMatrix = triangle->get_model_matrix();
	vkCmdPushConstants(frameCmd->get_handle(), m_graphicPipeline->get_pipeline_layout()->get_vk_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(gmat4), &modelMatrix);

	vkCmdDraw(frameCmd->get_handle(), 3, 1, 0, 0);*/


	vp->end_draw_cmd(frameCmd);

	vp->begin_composition_draw_cmd(frameCmd);
	deferredRenderer->fill_composition_cmd(frameCmd, currIndex);

	m_cubemapRenderer->render(frameCmd, currIndex,vp);

	auto sceneWindow = EditorApplicationImpl::get_instance()->get_editor_layer()->get_scene_window();
	if (auto selectedNode = sceneWindow->get_selected_entity(); selectedNode != -1)
	{
		auto drawId = m_sceneManager->get_draw_id_of_node(selectedNode);
		if (drawId != -1)
		{
			deferredRenderer->fill_aabb_cmd_for(frameCmd, currIndex, selectedNode);
		}
	}

	if (m_gridRenderer->wants_render())
	{
		m_gridRenderer->render(frameCmd, currIndex);
	}
	vp->end_draw_cmd(frameCmd);
	frameCmd->end();

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

	//m_currentCmdIndex[currIndex] = (m_currentCmdIndex[currIndex] + 1) % perFrameCmd;
}

bool GSceneRenderer::init()
{
	EditorApplicationImpl::get_instance()->get_editor_logger()->log_d("Create the main cmd");
	auto frameCount = EditorApplicationImpl::get_instance()->m_engine->get_frame_count();

	for (int i = 0; i < frameCount; i++)
	{
		m_frameCmds.push_back(std::vector<GVulkanCommandBuffer*>());
		for (int j = 0; j < perFrameCmd; j++)
		{
			auto fdata = EditorApplicationImpl::get_instance()->m_engine->get_frame_data_by_index(i);
			auto cmd = fdata->create_command_buffer_for_this_frame();
			cmd->init();
			m_frameCmds[i].push_back(cmd);
		}
		m_currentCmdIndex.push_back(0);
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
	auto stageRes = s_shaderManager->create_shader_stage_from_shader_res(m_basicVertexShader.get());
	if (stageRes.has_value())
	{
		m_vertexShaderStage = stageRes.value();
	}
	else
	{
		assert(false);
	}

	auto stageRes2 = s_shaderManager->create_shader_stage_from_shader_res(m_basicFragShader.get());
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

	auto sceneMng = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();

	m_cubemapRenderer = GSharedPtr<GCubeRenderer>(new GCubeRenderer(s_device,resourceManager,
		((GSharedPtr<IGCameraManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get(), sceneMng,
		((GSharedPtr<IGPipelineObjectManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_PIPELINE_OBJECT))->get()
	,m_viewport, s_shaderManager,m_viewport->get_composition_renderpass(), m_frameCmds.size(), "assets/bgg.hdr"));

	m_gridRenderer = GSharedPtr<GridRenderer>(new GridRenderer(s_device, resourceManager,
		((GSharedPtr<IGCameraManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get(), sceneMng,
		((GSharedPtr<IGPipelineObjectManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_PIPELINE_OBJECT))->get()
		, m_viewport, s_shaderManager, m_viewport->get_composition_renderpass(), m_frameCmds.size()));

	assert(m_cubemapRenderer->init());
	assert(m_gridRenderer->init());

	auto win = new GImGuiGridSettingsWindow(m_gridRenderer.get());
	if (!EditorApplicationImpl::get_instance()->get_editor_layer()->get_window_manager()->create_imgui_window(win, GIMGUIWINDOWDIR_LEFT_BOTTOM))
	{
		delete win;
	}
	m_sceneManager = sceneMng;
	return true;
}

void GSceneRenderer::destroy()
{
	m_gridRenderer->destroy();
	m_cubemapRenderer->destroy();
	triangle->destroy();
	delete triangle;
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
		for (int j = 0; j < perFrameCmd; j++)
		{
			m_frameCmds[i][j]->destroy();
			delete m_frameCmds[i][j];

		}
		m_device->destroy_semaphore(m_frameSemaphores[i]);
	}
	
}

void GSceneRenderer::set_the_viewport(IGVulkanNamedDeferredViewport* viewport)
{
	m_viewport = viewport;
	
}
