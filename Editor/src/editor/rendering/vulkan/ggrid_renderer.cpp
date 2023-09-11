#include "volk.h"
#include "internal/rendering/vulkan/ggrid_renderer.h"
#include "engine/manager/igresource_manager.h"
#include "engine/resource/igshader_resource.h"
#include "internal/rendering/vulkan/gcube_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include "engine/manager/igshader_manager.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/rendering/vulkan/ggrid_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
GridRenderer::GridRenderer(IGVulkanLogicalDevice* boundedDevice, IGResourceManager* mng, IGCameraManager* cameraManager, IGPipelineObjectManager* obj,
	IGVulkanViewport* viewport, IGShaderManager* shaderMng, uint32_t framesInFlight)
{
	m_boundedDevice = boundedDevice;
	m_framesInFlight = framesInFlight;

	p_cameraManager = cameraManager;
	p_renderingViewport = viewport;
	p_shaderManager = shaderMng;
	p_pipelineManager = obj;

	m_framesInFlight = framesInFlight;
	m_spec.gridCellSize = 0.1f;
	m_gridFrag = mng->create_shader_resource("grid_frag","EditorResources","grid.glsl_frag").value();
	m_gridVert = mng->create_shader_resource("grid_vert", "EditorResources", "grid.glsl_vert").value();
}

bool GridRenderer::init()
{
	auto code = m_gridFrag->load();
	assert(code == RESOURCE_INIT_CODE_OK);
	code = m_gridVert->load();
	assert(code == RESOURCE_INIT_CODE_OK);
	
	m_gridVertStage = p_shaderManager->create_shader_stage_from_shader_res(m_gridVert).value();
	m_gridFragStage = p_shaderManager->create_shader_stage_from_shader_res(m_gridFrag).value();

	std::vector<IVulkanShaderStage*> stages(2);
	stages[0] = m_gridVertStage;
	stages[1] = m_gridFragStage;

	std::vector<IGVulkanGraphicPipelineState*> states;
	VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.blendEnable = true;
	colorAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT;
	colorAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorState = {};
	colorState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorState.logicOpEnable = VK_FALSE;
	colorState.logicOp = VK_LOGIC_OP_COPY;
	colorState.attachmentCount = 1;
	colorState.pAttachments = &colorAttachmentState;
	
	VkPipelineDepthStencilStateCreateInfo depthInfo = {};
	depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthInfo.pNext = nullptr;
	depthInfo.flags = 0;
	depthInfo.depthTestEnable = VK_FALSE;
	depthInfo.depthWriteEnable = VK_FALSE;
	depthInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthInfo.stencilTestEnable = false;
	depthInfo.minDepthBounds = 0;
	depthInfo.maxDepthBounds = 1;

	states.push_back(m_boundedDevice->create_vertex_input_state(nullptr, nullptr));
	states.push_back(m_boundedDevice->create_default_input_assembly_state());
	states.push_back(m_boundedDevice->create_default_rasterization_state());
	states.push_back(m_boundedDevice->create_default_none_multisample_state());
	states.push_back(m_boundedDevice->create_custom_color_blend_state(&colorAttachmentState,&colorState));
	states.push_back(m_boundedDevice->create_default_viewport_state(1920, 1080));
	states.push_back(m_boundedDevice->create_default_depth_stencil_state());

	//X Create the pipeline here
	m_pipelineLayoutCreator = new GGridPipelineLayoutCreator(m_boundedDevice, p_cameraManager, p_pipelineManager, m_framesInFlight);
	m_pipeline = m_boundedDevice->create_and_init_graphic_pipeline_injector_for_vp(p_renderingViewport, stages, states, m_pipelineLayoutCreator);

	for (int i = 0; i < states.size(); i++)
	{
		delete states[i];
	}

	return true;
}

void GridRenderer::destroy()
{
	m_pipeline->destroy();
	delete m_pipeline;


	delete m_gridFragStage;
	delete m_gridVertStage;
	m_gridVert->destroy();
	delete m_gridVert;
	m_gridFrag->destroy();
	delete m_gridFrag;
}

bool GridRenderer::wants_render() const noexcept
{
	return m_wantsRender;
}

void GridRenderer::render(GVulkanCommandBuffer* cmd, uint32_t frameIndex)
{
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->get_pipeline());

	m_pipeline->bind_sets(cmd, frameIndex);
	const float* pos = p_cameraManager->get_camera_position();

	auto mat = transform.to_mat4();

	vkCmdSetViewport(cmd->get_handle(), 0, 1, p_renderingViewport->get_viewport_area());
	vkCmdSetScissor(cmd->get_handle(), 0, 1, p_renderingViewport->get_scissor_area());
	vkCmdPushConstants(cmd->get_handle(), m_pipeline->get_pipeline_layout()->get_vk_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(gmat4), &mat);
	vkCmdPushConstants(cmd->get_handle(), m_pipeline->get_pipeline_layout()->get_vk_pipeline_layout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(gmat4), sizeof(GridSpec), &m_spec);
	vkCmdDraw(cmd->get_handle(), 6, 1, 0, 0);
}

GridSpec* GridRenderer::get_spec() noexcept
{
	return &m_spec;
}

