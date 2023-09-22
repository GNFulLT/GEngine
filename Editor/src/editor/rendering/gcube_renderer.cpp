#include "volk.h"
#include "internal/rendering/vulkan/gcube_renderer.h"
#include "engine/manager/igresource_manager.h"
#include "engine/resource/igtexture_resource.h"
#include <unordered_map>
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/rendering/vulkan/gcube_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include "engine/manager/igshader_manager.h"
#include "engine/resource/igshader_resource.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include <glm/glm.hpp>
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/manager/igcamera_manager.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>

GCubeRenderer::GCubeRenderer(IGVulkanLogicalDevice* boundedDevice, IGResourceManager* mng,IGCameraManager* cameraManager, IGSceneManager* sceneManager, IGPipelineObjectManager* obj,IGVulkanViewport* viewport,IGShaderManager* shaderMng,uint32_t frameInFlight, const char* cubeTexturePath)
{
	p_sceneManager = sceneManager;
	m_obj = obj;
	m_pipeline = nullptr;
	m_cubemapFragStage = nullptr;
	m_cubemapVertexStage = nullptr;
	m_shaderManager = shaderMng;
	m_viewport = viewport;
	p_cameraManager = cameraManager;
	m_pipelineCreator = nullptr;
	m_framesInFlight = frameInFlight;
	m_boundedDevice = boundedDevice;
	auto loader = mng->get_imageloader_with_name("cubemap_loader");
	assert(loader != nullptr);
	auto res = mng->create_texture_resource("CubeMap","EditorResources",cubeTexturePath,nullptr,loader,VkFormat::VK_FORMAT_R8G8B8A8_SRGB);
	if (res.has_value())
	{
		m_cubemapTextureResource = GSharedPtr<IGTextureResource>(res.value());
	}
	auto shaderRes = mng->create_shader_resource("CubeMapFragShader", "EditorResources", "./cubemap.glsl_vert");
	if (shaderRes.has_value())
	{
		m_cubemapVertexShader = GSharedPtr<IGShaderResource>(shaderRes.value());
	}
	shaderRes = mng->create_shader_resource("CubeMapFragShader", "EditorResources", "./cubemap.glsl_frag");
	if (shaderRes.has_value())
	{
		m_cubemapFragShader = GSharedPtr<IGShaderResource>(shaderRes.value());
	}
	cubeTransform.scale = gvec3(150, 150, 150);
}

bool GCubeRenderer::init()
{
	if (!m_cubemapTextureResource.is_valid() || !m_cubemapVertexShader.is_valid() || !m_cubemapFragShader.is_valid())
		return false;

	
	auto inited = m_cubemapTextureResource->load();
	if (inited != RESOURCE_INIT_CODE_OK)
		return false;

	inited = m_cubemapVertexShader->load();
	if (inited != RESOURCE_INIT_CODE_OK)
	{
		destroy();
		return false;
	}

	inited = m_cubemapFragShader->load();
	if (inited != RESOURCE_INIT_CODE_OK)
	{
		destroy();
		return false;
	}

	
	m_pipelineCreator = new GCubePipelinelayoutCreator(m_boundedDevice,p_sceneManager,m_obj,m_cubemapTextureResource,m_framesInFlight);
	//X TODO : CHANAGE TO THE RENDERPASS NOT VIEWPORT
	
	m_cubemapFragStage = m_shaderManager->create_shader_stage_from_shader_res(m_cubemapFragShader.get()).value();
	m_cubemapVertexStage = m_shaderManager->create_shader_stage_from_shader_res(m_cubemapVertexShader.get()).value();

	std::vector<IVulkanShaderStage*> stages(2);
	stages[0] = m_cubemapVertexStage;
	stages[1] = m_cubemapFragStage;

	std::vector<IGVulkanGraphicPipelineState*> states;
	states.push_back(m_boundedDevice->create_vertex_input_state(nullptr, nullptr));
	states.push_back(m_boundedDevice->create_default_input_assembly_state());
	states.push_back(m_boundedDevice->create_default_rasterization_state());
	states.push_back(m_boundedDevice->create_default_none_multisample_state());
	states.push_back(m_boundedDevice->create_default_color_blend_state());
	states.push_back(m_boundedDevice->create_default_viewport_state(1920, 1080));
	states.push_back(m_boundedDevice->create_default_depth_stencil_state());

	m_pipeline = m_boundedDevice->create_and_init_graphic_pipeline_injector_for_vp(m_viewport,stages, states,m_pipelineCreator);
	for (int i = 0; i < states.size(); i++)
	{
		delete states[i];
	}

	if (m_pipeline == nullptr)
	{
		destroy();
		return false;
	}
	
	
}

void GCubeRenderer::destroy()
{
	if (m_cubemapVertexStage != nullptr)
	{
		delete m_cubemapFragStage;
		m_cubemapFragStage = nullptr;
	}
	if (m_cubemapVertexStage != nullptr)
	{
		delete m_cubemapVertexStage;
		m_cubemapVertexStage = nullptr;
	}
	if (m_pipeline != nullptr)
	{
		m_pipeline->destroy();
		delete m_pipeline;
	}
	if (m_cubemapFragShader.is_valid())
	{
		m_cubemapFragShader->destroy();
	}
	if (m_cubemapVertexShader.is_valid())
	{
		m_cubemapVertexShader->destroy();
	}
	if (m_cubemapTextureResource.is_valid())
	{
		m_cubemapTextureResource->destroy();
	}
}

void GCubeRenderer::render(GVulkanCommandBuffer* buff, uint32_t frameIndex,IGVulkanViewport* vp)
{

	vkCmdBindPipeline(buff->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,m_pipeline->get_pipeline());

	m_pipeline->bind_sets(buff, frameIndex);
	const float* pos = p_cameraManager->get_camera_position();
	cubeTransform.position = gvec3(0,0,0);
	auto mat = cubeTransform.to_mat4();

	vkCmdSetViewport(buff->get_handle(), 0, 1, vp->get_viewport_area());
	vkCmdSetScissor(buff->get_handle(), 0, 1, vp->get_scissor_area());
	vkCmdPushConstants(buff->get_handle(),m_pipeline->get_pipeline_layout()->get_vk_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mat);

	vkCmdDraw(buff->get_handle(), 36, 1, 0, 0);
}
