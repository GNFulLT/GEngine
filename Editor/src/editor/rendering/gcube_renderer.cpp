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
#include <array>

GCubeRenderer::GCubeRenderer(IGVulkanLogicalDevice* boundedDevice, IGResourceManager* mng,IGCameraManager* cameraManager, IGSceneManager* sceneManager,
	IGPipelineObjectManager* obj, IGVulkanNamedViewport* viewport,IGShaderManager* shaderMng, IGVulkanNamedRenderPass* renderpass,uint32_t frameInFlight, const char* cubeTexturePath)
{
	m_renderpass = renderpass;
	p_sceneManager = sceneManager;
	m_obj = obj;
	m_pipeline = nullptr;
	m_cubemapFragStage = nullptr;
	m_cubemapVertexStage = nullptr;
	m_shaderManager = shaderMng;
	m_viewport = viewport;
	p_cameraManager = cameraManager;
	m_framesInFlight = frameInFlight;
	m_boundedDevice = boundedDevice;
	auto loader = mng->get_imageloader_with_name("cubemap_loader");
	assert(loader != nullptr);
	auto res = mng->create_texture_resource("CubeMap","EditorResources",cubeTexturePath,nullptr,loader,VkFormat::VK_FORMAT_R8G8B8A8_SRGB);
	if (res.has_value())
	{
		m_cubemapTextureResource = GSharedPtr<IGTextureResource>(res.value());
	}
	auto shaderRes = mng->create_shader_resource("CubeMapFragShader", "EditorResources", "./data/shader/cubemap.glsl_vert");
	if (shaderRes.has_value())
	{
		m_cubemapVertexShader = GSharedPtr<IGShaderResource>(shaderRes.value());
	}
	shaderRes = mng->create_shader_resource("CubeMapFragShader", "EditorResources", "./data/shader/cubemap.glsl_frag");
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
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		//X Create only texture set layout
		VkDescriptorSetLayoutCreateInfo info = {};
		info.bindingCount = 1;
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pBindings = &binding;
		
		m_csLayout = m_obj->create_or_get_named_set_layout("cs1f",&info);
		assert(m_csLayout != nullptr);

		//X Create set
		std::unordered_map<VkDescriptorType, int> types;
		types.emplace(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1);

		m_csPool = m_boundedDevice->create_and_init_vector_pool(types, 1);
		auto vkLayout = m_csLayout->get_layout();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &vkLayout;
		allocInfo.descriptorPool = m_csPool->get_vk_descriptor_pool();

		auto vkRes = vkAllocateDescriptorSets(m_boundedDevice->get_vk_device(), &allocInfo, &m_csSet);
		assert(VK_SUCCESS == vkRes);

		//X Write set
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_cubemapTextureResource->get_vulkan_image()->get_vk_image_view();
		imageInfo.sampler = m_obj->get_named_sampler(m_obj->MAX_PERFORMANT_SAMPLER.data())->get_vk_sampler();

		VkWriteDescriptorSet writeSet = {};
		writeSet.descriptorCount = 1;
		writeSet.dstBinding = 0;
		writeSet.pImageInfo = &imageInfo;
		writeSet.dstSet = m_csSet;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

		vkUpdateDescriptorSets(m_boundedDevice->get_vk_device(), 1, &writeSet, 0, 0);
		
	}
	{
		auto globalDataLayout = m_obj->get_named_set_layout("GlobalDataSetLayout");
		assert(globalDataLayout != nullptr);
		std::array<VkDescriptorSetLayout, 2> setLayouts;
		setLayouts[0] = globalDataLayout->get_layout();
		setLayouts[1] = m_csLayout->get_layout();

		VkPipelineLayoutCreateInfo inf = {};
		inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		inf.flags = 0;
		inf.setLayoutCount = setLayouts.size();
		inf.pSetLayouts = setLayouts.data();
		inf.pushConstantRangeCount = 0;
		inf.pPushConstantRanges = nullptr;

		m_pipeLayout = m_obj->create_or_get_named_pipeline_layout("GlobalData_cs1f",&inf);
	}
	this->m_pipeline = m_obj->create_named_graphic_pipeline("cube_renderer",m_renderpass);
	assert(m_pipeline->init(m_pipeLayout,stages,states));

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
	if (m_csPool != nullptr)
	{
		m_csPool->destroy();
		delete m_csPool;
	}
}

void GCubeRenderer::render(GVulkanCommandBuffer* buff, uint32_t frameIndex,IGVulkanNamedViewport* vp)
{

	vkCmdBindPipeline(buff->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,m_pipeline->get_vk_pipeline());
	auto lf = p_sceneManager->get_global_set_for_frame(frameIndex);
	std::array<VkDescriptorSet,2> sets;
	sets[0] = lf;
	sets[1] = m_csSet;

	vkCmdBindDescriptorSets(buff->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,m_pipeLayout->get_vk_pipeline_layout(), 0,
		sets.size(), sets.data(), 0, 0);

	const float* pos = p_cameraManager->get_camera_position();
	cubeTransform.position = gvec3(0,0,0);
	auto mat = cubeTransform.to_mat4();

	vkCmdSetViewport(buff->get_handle(), 0, 1, vp->get_viewport_area());
	vkCmdSetScissor(buff->get_handle(), 0, 1, vp->get_scissor_area());

	vkCmdDraw(buff->get_handle(), 36, 1, 0, 0);
}
