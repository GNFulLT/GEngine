#include "volk.h"
#include "internal/engine/rendering/renderer/gscene_renderer2.h"
#include <array>
#include <vma/vk_mem_alloc.h>
#include <unordered_map>
#include "engine/resource/igshader_resource.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "engine/rendering/vulkan/named/igvulkan_named_sampler.h"
#include "engine/rendering/vulkan/named/igvulkan_named_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_pdevice.h"
#include "engine/gengine.h"
#include "engine/io/iowning_glogger.h"
#include "internal/engine/manager/glogger_manager.h"
#include <spdlog/fmt/fmt.h>
#include "engine/rendering/scene/scene.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "engine/imanager_table.h"
#include "engine/rendering/vulkan/transfer/itransfer_op.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define SHADOWMAP_DIM 2048
#define SHADOWMAP_FORMAT VK_FORMAT_D32_SFLOAT
#define JITTER_WINDOW_SIZE 16
#define JITTER_FILTER_SIZE 8

#define POSITION_TARGET_COLOR_FORMAT VK_FORMAT_R32G32B32A32_SFLOAT
#define EMISSION_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define ALBEDO_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define PBR_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define DEPTH_TARGET_COLOR_FORMAT VK_FORMAT_D32_SFLOAT

struct AABBDrawer
{
	uint32_t meshId;
	glm::mat4 transform;
};

GSceneRenderer2::GSceneRenderer2(IGVulkanLogicalDevice* dev, IGPipelineObjectManager* pipelineManager, IGResourceManager* res, IGShaderManager* shaderMng, IGSceneManager* sceneMng,
	uint32_t framesInFlight, VkFormat compositionFormat)
{
	p_sceneManager = sceneMng;
	p_boundedDevice = dev;
	p_resourceManager = res;
	p_shaderManager = shaderMng;
	p_pipelineManager = pipelineManager;
	m_framesInFlight = framesInFlight;
	m_compositionFormat = compositionFormat;
	m_meshletStreamResources = nullptr;
}

bool GSceneRenderer2::init(VkDescriptorSetLayout_T* globalUniformSet, IGVulkanNamedSetLayout* drawDataSetLayout, IGVulkanNamedSetLayout* lightDataSetLayout, IGVulkanNamedSetLayout* cullSetLayout)
{
	m_meshStreamResources = new GPUMeshStreamResources(p_boundedDevice, 7, m_framesInFlight, p_pipelineManager);
	
	m_useMeshlet = true && p_boundedDevice->has_meshlet_support();

	uint32_t beginMesh = calculate_nearest_1mb<GMeshData>();
	assert(m_meshStreamResources->init(calculate_nearest_10mb<float>()*3, calculate_nearest_10mb<uint32_t>()*14, beginMesh,
		calculate_nearest_1mb<DrawData>()));


	if (m_useMeshlet)
	{
		m_meshletStreamResources = new GPUMeshletStreamResources(p_boundedDevice, p_pipelineManager, m_framesInFlight);
		m_meshletStreamResources->init(beginMesh, beginMesh * 50, beginMesh * 64, beginMesh * 126);
	}
	//X Create DrawDataSet
	{
		//X Bindless Texture 
		{
			auto maxImage = p_boundedDevice->get_bounded_physical_device()->get_vk_properties()->limits.maxDescriptorSetSampledImages;
			auto maxStoredImage = p_boundedDevice->get_bounded_physical_device()->get_vk_properties()->limits.maxDescriptorSetStorageImages;
			if (maxImage == -1)
			{
				GLoggerManager::get_instance()->log_e("GSceneRenderer", fmt::format("Your maximum support is {} so it is -1 wtf", maxImage).c_str());
			}
			if (maxStoredImage == -1)
			{
				GLoggerManager::get_instance()->log_e("GSceneRenderer", fmt::format("Your maximum storage image support is {} so it is -1 wtf", maxImage).c_str());
			}
			else
			{
				if (maxImage > MAX_BINDLESS_TEXTURE)
				{
					GLoggerManager::get_instance()->log_d("GSceneRenderer", fmt::format("Your maximum sampled support was {}. But engine will use {}", maxImage, MAX_BINDLESS_TEXTURE).c_str());
				}
				else
				{
					GLoggerManager::get_instance()->log_d("GSceneRenderer", fmt::format("Engine wanted to use {}. But you maximum sampled support was {}. Please buy a new GPU immediately", MAX_BINDLESS_TEXTURE, maxImage).c_str());
					MAX_BINDLESS_TEXTURE = maxImage;
				}

				if (maxStoredImage > MAX_BINDLESS_TEXTURE)
				{
					GLoggerManager::get_instance()->log_d("GSceneRenderer", fmt::format("Your maximum stored image support was {}. But engine will use {}", maxStoredImage, MAX_BINDLESS_TEXTURE).c_str());
				}
				else
				{
					GLoggerManager::get_instance()->log_d("GSceneRenderer", fmt::format("Engine wanted to use {}. But you maximum storage image support was {}. Please buy a new GPU immediately", MAX_BINDLESS_TEXTURE, maxStoredImage).c_str());
					MAX_BINDLESS_TEXTURE = maxStoredImage;
				}
			}
			

			VkDescriptorPoolSize pool_sizes_bindless[] =
			{
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				  MAX_BINDLESS_TEXTURE },
				  { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				  MAX_BINDLESS_TEXTURE },
			};
			
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
			pool_info.maxSets = MAX_BINDLESS_TEXTURE * 2;
			pool_info.poolSizeCount = 2;
			pool_info.pPoolSizes = pool_sizes_bindless;
			assert(VK_SUCCESS == vkCreateDescriptorPool(p_boundedDevice->get_vk_device(), &pool_info,
				nullptr,
				&m_bindlessPool));


			std::array<VkDescriptorSetLayoutBinding, 2> bindings;
			//X TEXTURE
			bindings[0].binding = 0;
			bindings[0].descriptorCount = MAX_BINDLESS_TEXTURE;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[0].pImmutableSamplers = nullptr;

			//X Image Buff
			bindings[1].binding =  1;
			bindings[1].descriptorCount = MAX_BINDLESS_TEXTURE;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[1].pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			layout_info.bindingCount = bindings.size();
			layout_info.pBindings = bindings.data();
			layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
			//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
			VkDescriptorBindingFlags bindless_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
			VkDescriptorBindingFlags binding_flags[2];
			binding_flags[0] = bindless_flags;
			binding_flags[1] = bindless_flags;

			VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr };
			extended_info.bindingCount = bindings.size();
			extended_info.pBindingFlags = binding_flags;

			layout_info.pNext = &extended_info;

			assert(VK_SUCCESS == vkCreateDescriptorSetLayout(p_boundedDevice->get_vk_device(), &layout_info, nullptr, &m_bindlessSetLayout));
			VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			alloc_info.descriptorPool = m_bindlessPool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &m_bindlessSetLayout;

			VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
			uint32_t max_binding = MAX_BINDLESS_TEXTURE - 1;
			count_info.descriptorSetCount = 1;
			// This number is the max allocatable count
			count_info.pDescriptorCounts = &max_binding;
			alloc_info.pNext = &count_info;
			auto allocRes = vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &alloc_info, &m_bindlessSet);
			if (allocRes != VK_SUCCESS)
			{
				GLoggerManager::get_instance()->log_c("GSceneRenderer", fmt::format("Allocating bindless texture failed with code {}", (int)allocRes).c_str());
			}
			assert(VK_SUCCESS == allocRes);
		}
		//X Composition layout
		{
			//X First create pool
			{
				//X First create necessary pool
				std::unordered_map<VkDescriptorType, int> map;
				map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6);
				map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
				map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);

				//X We allocate three set
				m_compositionPool = p_boundedDevice->create_and_init_vector_pool(map, 4);

				std::array<VkDescriptorSetLayoutBinding, 4> bindings;
				//X Pos Buff
				bindings[0].binding = 0;
				bindings[0].descriptorCount = 1;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				bindings[0].pImmutableSamplers = nullptr;

				//X Albedo Buff
				bindings[1].binding = 1;
				bindings[1].descriptorCount = 1;
				bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				bindings[1].pImmutableSamplers = nullptr;

				//X	EMISSION BUFF
				bindings[2].binding = 2;
				bindings[2].descriptorCount = 1;
				bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				bindings[2].pImmutableSamplers = nullptr;

				//X	PBR BUFF
				bindings[3].binding = 3;
				bindings[3].descriptorCount = 1;
				bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				bindings[3].pImmutableSamplers = nullptr;

				VkDescriptorSetLayoutCreateInfo setinfo = {};
				setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				setinfo.pNext = nullptr;
				//we are going to have 1 binding
				setinfo.bindingCount = bindings.size();
				//no flags
				setinfo.flags = 0;
				//point to the camera buffer binding
				setinfo.pBindings = bindings.data();

				auto res = vkCreateDescriptorSetLayout(p_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_compositionSetLayout);
				assert(res == VK_SUCCESS);


				VkDescriptorSetAllocateInfo allocInfo = {};
				allocInfo.pNext = nullptr;
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_compositionPool->get_vk_descriptor_pool();
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &m_compositionSetLayout;

				assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, &m_compositionSet));
			}

		}
	}
	{
		//X Sun Shadow Buffer
		std::array<VkDescriptorSetLayoutBinding, 2> bindings;
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
		bindings[0].pImmutableSamplers = nullptr;

		bindings[1].binding = 1;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
		bindings[1].pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pNext = nullptr;
		setinfo.bindingCount = bindings.size();
		setinfo.flags = 0;
		setinfo.pBindings = bindings.data();

		m_sunShadowSetLayout = p_pipelineManager->create_or_get_named_set_layout("SunShadowSetLayout", &setinfo);
	}
	//X Create pipeline layouts
	{
		//X DeferredLet Layout
		{
			VkPipelineLayoutCreateInfo inf = {};
			std::array<VkDescriptorSetLayout, 5> setLayouts;
			setLayouts[0] = globalUniformSet;
			setLayouts[1] = m_meshStreamResources->get_draw_set_layout()->get_layout();
			setLayouts[2] = drawDataSetLayout->get_layout();
			setLayouts[3] = m_bindlessSetLayout;
			setLayouts[4] = m_meshletStreamResources->get_meshlet_set_layout()->get_layout();

			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 0;
			inf.pPushConstantRanges = nullptr;

			m_deferredletLayout = p_pipelineManager->create_or_get_named_pipeline_layout("DeferredletPipelineLayout", &inf);
			assert(m_deferredletLayout != nullptr);
			
		}
		//X Deferred Layout
		{
			VkPipelineLayoutCreateInfo inf = {};
			std::array<VkDescriptorSetLayout, 4> setLayouts;
			setLayouts[0] = globalUniformSet;
			setLayouts[1] = m_meshStreamResources->get_draw_set_layout()->get_layout();
			setLayouts[2] = drawDataSetLayout->get_layout();
			setLayouts[3] = m_bindlessSetLayout;


			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 0;
			inf.pPushConstantRanges = nullptr;
			
			{
				setLayouts[1] = m_meshStreamResources->get_draw_set_layout()->get_layout();
				m_deferredLayout = p_pipelineManager->create_or_get_named_pipeline_layout("DeferredPipelineLayout", &inf);
				assert(m_deferredLayout != nullptr);
			}
			
	
			VkPushConstantRange range = {};
			range.offset = 0;
			range.size = sizeof(glm::mat4);
			range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			inf.pushConstantRangeCount = 1;
			inf.pPushConstantRanges = &range;

			m_sunShadowLayout = p_pipelineManager->create_or_get_named_pipeline_layout("SunShadowPipelineLayout", &inf);
			assert(m_sunShadowLayout != nullptr);

		}

		//X Composition layout
		{
			std::array<VkDescriptorSetLayout, 4> setLayouts;
			setLayouts[0] = m_compositionSetLayout;
			setLayouts[1] = lightDataSetLayout->get_layout();
			setLayouts[2] = globalUniformSet;
			setLayouts[3] = m_sunShadowSetLayout->get_layout();

			VkPipelineLayoutCreateInfo inf = {};
			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 0;
			inf.pPushConstantRanges = nullptr;

			m_compositionLayout = p_pipelineManager->create_or_get_named_pipeline_layout("CompositionPipelineLayout",&inf);
			assert(m_compositionLayout != nullptr);
		}
		//X Compute Layout
		{
			std::array<VkDescriptorSetLayout, 5> setLayouts;
			setLayouts[0] = globalUniformSet;
			setLayouts[1] = m_meshStreamResources->get_indirect_set_layout()->get_layout();
			setLayouts[2] = m_meshStreamResources->get_draw_set_layout()->get_layout();
			setLayouts[3] = drawDataSetLayout->get_layout();
			setLayouts[4] = cullSetLayout->get_layout();

			VkPipelineLayoutCreateInfo inf = {};
			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 0;
			inf.pPushConstantRanges = nullptr;

			m_computePipelineLayout = p_pipelineManager->create_or_get_named_pipeline_layout("ComputePipelineLayout", &inf);
			assert(m_computePipelineLayout != nullptr);
		}
		{
			std::array<VkDescriptorSetLayout, 6> setLayouts;
			setLayouts[0] = globalUniformSet;
			setLayouts[1] = m_meshStreamResources->get_indirect_set_layout()->get_layout();
			setLayouts[2] = m_meshStreamResources->get_draw_set_layout()->get_layout();
			setLayouts[3] = drawDataSetLayout->get_layout();
			setLayouts[4] = cullSetLayout->get_layout();
			setLayouts[5] = m_meshletStreamResources->get_meshlet_set_layout()->get_layout();

			VkPipelineLayoutCreateInfo inf = {};
			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 0;
			inf.pPushConstantRanges = nullptr;

			m_computeMeshletPipelineLayout = p_pipelineManager->create_or_get_named_pipeline_layout("ComputeMeshletPipelineLayout", &inf);
			assert(m_computeMeshletPipelineLayout != nullptr);
		}

		//X AABB Draw Layout
		{
			std::array<VkDescriptorSetLayout, 3> setLayouts;
			setLayouts[0] = globalUniformSet;
			setLayouts[1] = m_meshStreamResources->get_draw_set_layout()->get_layout();
			setLayouts[2] = drawDataSetLayout->get_layout();

			VkPushConstantRange range = {};
			range.offset = 0;
			range.size = sizeof(AABBDrawer);
			range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkPipelineLayoutCreateInfo inf = {};
			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 1;
			inf.pPushConstantRanges = &range;

			m_aabbDrawLayout = p_pipelineManager->create_or_get_named_pipeline_layout("BoundingBoxPipelineLayout", &inf);
			assert(m_aabbDrawLayout != nullptr);
		}
	}
	//X Create the renderpass
	{
		{
			//X First pass
			std::array<VkAttachmentDescription, 5> attachmentDescs = {};
			for (uint32_t i = 0; i < 5; ++i)
			{
				attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				//X Last one will be for depth attachment
				if (i == 4)
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
			}
			attachmentDescs[0].format = POSITION_TARGET_COLOR_FORMAT;
			attachmentDescs[1].format = ALBEDO_TARGET_COLOR_FORMAT;
			attachmentDescs[2].format = EMISSION_TARGET_COLOR_FORMAT;
			attachmentDescs[3].format = PBR_TARGET_COLOR_FORMAT;
			attachmentDescs[4].format = DEPTH_TARGET_COLOR_FORMAT;

			std::vector<VkAttachmentReference> colorReferences;
			colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			colorReferences.push_back({ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 4;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pColorAttachments = colorReferences.data();
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
			subpass.pDepthStencilAttachment = &depthReference;

			// Use subpass dependencies for attachment layout transitions
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = 0;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pAttachments = attachmentDescs.data();
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 2;
			renderPassInfo.pDependencies = dependencies.data();

			m_deferredPass = p_pipelineManager->create_or_get_named_renderpass("deferred_pass", &renderPassInfo);
			assert(m_deferredPass != nullptr);
		}
		//X Composition Renderpass
		{
			std::array<VkAttachmentDescription, 2> attachmentDescs = {};
			for (uint32_t i = 0; i < 2; ++i)
			{
				attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				//X Last one will be for depth attachment
				if (i == 1)
				{
					//X Dont clear the depth buffer
					attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
			}
			attachmentDescs[0].format = m_compositionFormat;
			attachmentDescs[1].format = DEPTH_TARGET_COLOR_FORMAT;


			std::vector<VkAttachmentReference> colorReferences;
			colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 1;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pColorAttachments = colorReferences.data();
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
			subpass.pDepthStencilAttachment = &depthReference;

			// Use subpass dependencies for attachment layout transitions
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = 0;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pAttachments = attachmentDescs.data();
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 2;
			renderPassInfo.pDependencies = dependencies.data();
			m_compositionPass = p_pipelineManager->create_or_get_named_renderpass("composition_pass", &renderPassInfo);
			assert(m_compositionPass != nullptr);
		}
	}
	//X Create shader resources
	{
		m_deferredVertexShaderRes = p_resourceManager->create_shader_resource("deferredVertex", "SceneRenderer", "./data/shader/deferred.glsl_vert").value();
		m_deferredFragmentShaderRes = p_resourceManager->create_shader_resource("deferredFragment", "SceneRenderer", "./data/shader/deferred.glsl_frag").value();
		m_deferredFragmentPBRShaderRes = p_resourceManager->create_shader_resource("deferredFragment", "SceneRenderer", "./data/shader/composition_pbr.glsl_frag").value();
		m_deferredletVertexShaderRes = p_resourceManager->create_shader_resource("deferredVertex", "SceneRenderer", "./data/shader/deferred_meshlet.glsl_vert").value();
		//X This will be loaded when mesh shading enable
		if (m_useMeshlet)
		{
			m_deferredTaskShaderRes = p_resourceManager->create_shader_resource("deferredVertex", "SceneRenderer", "./data/shader/meshlet_task.glsl_task").value();
			m_deferredMeshShaderRes = p_resourceManager->create_shader_resource("deferredVertex", "SceneRenderer", "./data/shader/meshlet_task.glsl_mesh").value();
			m_deferredMeshFragmentShaderRes = p_resourceManager->create_shader_resource("deferredMeshFrag", "SceneRenderer", "./data/shader/deferred_meshlet.glsl_frag").value();
			m_sunShadowMeshShaderRes = p_resourceManager->create_shader_resource("sunShadowVertex", "SceneRenderer", "./data/shader/sun_shadow.glsl_mesh").value();
			m_sunShadowTaskShaderRes = p_resourceManager->create_shader_resource("sunShadowVertex", "SceneRenderer", "./data/shader/sun_shadow.glsl_task").value();

		}
		
		m_compositionVertexShaderRes = p_resourceManager->create_shader_resource("compositionVertex", "SceneRenderer", "./data/shader/composition.glsl_vert").value();
		m_compositionFragmentShaderRes = p_resourceManager->create_shader_resource("compositionFrag", "SceneRenderer", "./data/shader/composition.glsl_frag").value();
		m_cullComputeShaderRes = p_resourceManager->create_shader_resource("cullCompute", "SceneRenderer", "./data/shader/cull.glsl_comp").value();
		m_cullComputeMeshletShaderRes = p_resourceManager->create_shader_resource("cullCompute", "SceneRenderer", "./data/shader/cull_meshlet.glsl_comp").value();

		m_boundingBoxVertexShaderRes = p_resourceManager->create_shader_resource("boundingBoxVertex", "SceneRenderer", "./data/shader/bounding_box.glsl_vert").value();
		m_boundingBoxFragmentShaderRes = p_resourceManager->create_shader_resource("boundingBoxFrag", "SceneRenderer", "./data/shader/bounding_box.glsl_frag").value();
		
		m_sunShadowVertexShaderRes = p_resourceManager->create_shader_resource("sunShadowVertex", "SceneRenderer", "./data/shader/sun_shadow.glsl_vert").value();
		m_sunShadowFragmentShaderRes = p_resourceManager->create_shader_resource("sunShadowVertex", "SceneRenderer", "./data/shader/sun_shadow.glsl_frag").value();

	
		assert(m_deferredVertexShaderRes->load() == RESOURCE_INIT_CODE_OK);
		//assert(m_deferredletVertexShaderRes->load() == RESOURCE_INIT_CODE_OK);

		assert(m_deferredFragmentShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_deferredFragmentPBRShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_compositionVertexShaderRes->load() == RESOURCE_INIT_CODE_OK);

		assert(m_compositionFragmentShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_cullComputeShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_cullComputeMeshletShaderRes->load() == RESOURCE_INIT_CODE_OK);

		assert(m_boundingBoxVertexShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_boundingBoxFragmentShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_sunShadowVertexShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_sunShadowFragmentShaderRes->load() == RESOURCE_INIT_CODE_OK);

		if (m_useMeshlet)
		{
			assert(m_sunShadowMeshShaderRes->load() == RESOURCE_INIT_CODE_OK);
			assert(m_sunShadowTaskShaderRes->load() == RESOURCE_INIT_CODE_OK);
			assert(m_deferredTaskShaderRes->load() == RESOURCE_INIT_CODE_OK);
			assert(m_deferredMeshShaderRes->load() == RESOURCE_INIT_CODE_OK);
			assert(m_deferredMeshFragmentShaderRes->load() == RESOURCE_INIT_CODE_OK);
		}
	}
	assert(load_shadow_resources());
	//X Create the pipeline
	{
		//X First deferred pipeline
		{
			std::vector<IGVulkanGraphicPipelineState*> states;

			std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(float) * 7;
			bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].offset = 0;

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].offset = sizeof(float) * 3;

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].offset = sizeof(float) * 5;

			std::array< VkPipelineColorBlendAttachmentState, 4> attachmentStates;
			attachmentStates[0] = {};
			attachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			attachmentStates[0].blendEnable = VK_FALSE;

			attachmentStates[1] = {};
			attachmentStates[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			attachmentStates[1].blendEnable = VK_FALSE;
			


			attachmentStates[2] = {};
			attachmentStates[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			attachmentStates[2].blendEnable = VK_FALSE;

			attachmentStates[3] = {};
			attachmentStates[3].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			attachmentStates[3].blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo	bcreateInfo = {};
			bcreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			bcreateInfo.logicOpEnable = VK_FALSE;
			bcreateInfo.logicOp = VK_LOGIC_OP_COPY;
			bcreateInfo.attachmentCount = attachmentStates.size();
			bcreateInfo.pAttachments = attachmentStates.data();

			VkPipelineDepthStencilStateCreateInfo dsInfo = {};
			dsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			dsInfo.pNext = nullptr;
			dsInfo.flags = 0;
			dsInfo.depthTestEnable = VK_TRUE;
			dsInfo.depthWriteEnable = VK_FALSE;
			dsInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			dsInfo.stencilTestEnable = false;
			dsInfo.minDepthBounds = 0;
			dsInfo.maxDepthBounds = 1;

			VkPipelineRasterizationStateCreateInfo	rasterInfo = {};
			rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterInfo.depthClampEnable = VK_FALSE;
			rasterInfo.rasterizerDiscardEnable = VK_FALSE;

			rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterInfo.lineWidth = 1.0f;
			// Cull front face
			rasterInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
			rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			//no depth bias
			rasterInfo.depthBiasEnable = VK_TRUE;
			rasterInfo.depthBiasConstantFactor = 1.25f;
			rasterInfo.depthBiasClamp = 0.0f;
			rasterInfo.depthBiasSlopeFactor = 1.75f;

			states.push_back(p_boundedDevice->create_vertex_input_state(nullptr, nullptr));
			states.push_back(p_boundedDevice->create_default_input_assembly_state());
			states.push_back(p_boundedDevice->create_default_none_multisample_state());
			states.push_back(p_boundedDevice->create_custom_color_blend_state(attachmentStates.data(), &bcreateInfo));
			states.push_back(p_boundedDevice->create_default_viewport_state(1920, 1080));
			states.push_back(p_boundedDevice->create_default_depth_stencil_state());
			states.push_back(p_boundedDevice->create_default_rasterization_state());
			

			//X Create Shader Stages
			std::vector<IVulkanShaderStage*> stages(2);
			stages[0] = p_shaderManager->create_shader_stage_from_shader_res(m_deferredVertexShaderRes).value();
			stages[1] = p_shaderManager->create_shader_stage_from_shader_res(m_deferredFragmentShaderRes).value();

			m_deferredPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_deferredPass, "deferred_pipeline");
			m_deferredPipeline->init(m_deferredLayout,stages,states);
		

			delete stages[0];
			delete stages[1];

			if (m_useMeshlet)
			{
				stages[0] = p_shaderManager->create_shader_stage_from_shader_res(m_deferredTaskShaderRes).value();
				stages[1] = p_shaderManager->create_shader_stage_from_shader_res(m_deferredMeshShaderRes).value();
				stages.push_back(p_shaderManager->create_shader_stage_from_shader_res(m_deferredFragmentShaderRes).value());

				m_deferredMeshletPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_deferredPass, "deferred_meshlet_pipeline");
				m_deferredMeshletPipeline->init(m_deferredletLayout, stages, states);
				delete stages[0];
				delete stages[1];
				delete stages[2];

				stages.resize(2);
			}
		
			delete states[3];
			delete states[6];

			stages[0] = p_shaderManager->create_shader_stage_from_shader_res(m_sunShadowVertexShaderRes).value();
			stages[1] = p_shaderManager->create_shader_stage_from_shader_res(m_sunShadowFragmentShaderRes).value();

			bcreateInfo.attachmentCount = 0;
			bcreateInfo.pAttachments = nullptr;
			states[6] = p_boundedDevice->create_custom_rasterization_state(&rasterInfo);

			states[3] = p_boundedDevice->create_custom_color_blend_state(attachmentStates.data(), &bcreateInfo);


			m_sunShadowPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_shadowPass, "sun_shadow_pipeline");
			m_sunShadowPipeline->init(m_sunShadowLayout, stages, states);

			//X TODO : SEPARATE 
			if (m_useMeshlet)
			{
				delete stages[0];
				stages[0] = p_shaderManager->create_shader_stage_from_shader_res(m_sunShadowMeshShaderRes).value();
				stages.resize(3);
				stages[2] = p_shaderManager->create_shader_stage_from_shader_res(m_sunShadowTaskShaderRes).value();
				m_sunShadowMeshPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_shadowPass, "sun_shadow_mesh_pipeline");
				m_sunShadowMeshPipeline->init(m_deferredletLayout, stages, states);
				delete stages[2];
				stages.resize(2);

			}
			
			delete stages[0];
			delete stages[1];
			delete states[6];

			states[6] = p_boundedDevice->create_default_rasterization_state();
			stages[0] = p_shaderManager->create_shader_stage_from_shader_res(m_compositionVertexShaderRes).value();
			stages[1] = p_shaderManager->create_shader_stage_from_shader_res(m_compositionFragmentShaderRes).value();
			delete states[3];
			delete states[5];
			states[3] = p_boundedDevice->create_default_color_blend_state();
			states[5] = p_boundedDevice->create_custom_depth_stencil_state(&dsInfo);

			m_compositionPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_compositionPass, "composition_pipeline");
			m_compositionPipeline->init(m_compositionLayout, stages, states);

			m_selectedCompositionPipeline = m_compositionPipeline;
			delete stages[1];
			stages[1] = p_shaderManager->create_shader_stage_from_shader_res(m_deferredFragmentPBRShaderRes).value();

			m_compositionPBRPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_compositionPass, "composition_pbr_pipeline");
			m_compositionPBRPipeline->init(m_compositionLayout, stages, states);

			delete stages[0];
			delete stages[1];
			
			stages[0] = p_shaderManager->create_shader_stage_from_shader_res(m_boundingBoxVertexShaderRes).value();
			stages[1] = p_shaderManager->create_shader_stage_from_shader_res(m_boundingBoxFragmentShaderRes).value();

			delete states[0];
			delete states[6];
			delete states[5];

			VkPipelineRasterizationStateCreateInfo rasterizationAABB = {};
			rasterizationAABB.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationAABB.polygonMode = VK_POLYGON_MODE_LINE;
			rasterizationAABB.lineWidth = 3.0f;
			//no backface cull
			rasterizationAABB.cullMode = VK_CULL_MODE_NONE;
			rasterizationAABB.frontFace = VK_FRONT_FACE_CLOCKWISE;
			//no depth bias
			rasterizationAABB.depthBiasEnable = VK_FALSE;
			rasterizationAABB.depthBiasConstantFactor = 1.25f;
			rasterizationAABB.depthBiasClamp = 0.0f;
			rasterizationAABB.depthBiasSlopeFactor = 1.75f;
			dsInfo.depthTestEnable = false;


			states[0] = p_boundedDevice->create_vertex_input_state(nullptr, nullptr);
			states[5] = p_boundedDevice->create_custom_depth_stencil_state(&dsInfo);
			states[6] = p_boundedDevice->create_custom_rasterization_state(&rasterizationAABB);
			m_aabbPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_compositionPass, "boundingBoxPipeline");
			m_aabbPipeline->init(m_aabbDrawLayout, stages, states);


			for (int i = 0; i < states.size(); i++)
			{
				delete states[i];
			}
		}
		//X Create compute pipeline
		{
			auto cullCompStage = p_shaderManager->create_shader_stage_from_shader_res(m_cullComputeShaderRes).value();
			VkComputePipelineCreateInfo compInfo = {};
			compInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			compInfo.pNext = 0;
			compInfo.flags = 0;
			compInfo.stage = *cullCompStage->get_creation_info();
			compInfo.basePipelineHandle = 0;
			compInfo.basePipelineIndex = 0;
			compInfo.layout = m_computePipelineLayout->get_vk_pipeline_layout();

			auto cmpPipe = vkCreateComputePipelines(p_boundedDevice->get_vk_device(), nullptr, 1, &compInfo, nullptr, &m_compPipeline);
			assert(cmpPipe == VK_SUCCESS);
			delete cullCompStage;

			cullCompStage = p_shaderManager->create_shader_stage_from_shader_res(m_cullComputeMeshletShaderRes).value();
			compInfo.stage = *cullCompStage->get_creation_info();
			compInfo.layout = m_computeMeshletPipelineLayout->get_vk_pipeline_layout();

			cmpPipe = vkCreateComputePipelines(p_boundedDevice->get_vk_device(), nullptr, 1, &compInfo, nullptr, &m_compMeshletPipeline);
			assert(cmpPipe == VK_SUCCESS);

		}

	}
	return true;
}

void GSceneRenderer2::set_drawdata_set(VkDescriptorSet_T* drawDataSet)
{
	m_drawDataSet = drawDataSet;
}

void GSceneRenderer2::set_lightdata_set(VkDescriptorSet_T* lightDataSet)
{
	m_lightDataSet = lightDataSet;
}

void GSceneRenderer2::set_culldata_set(VkDescriptorSet_T* cullDataSet)
{
	m_cullDataSet = cullDataSet;
}

uint32_t GSceneRenderer2::get_max_indirect_draw_count()
{
	return m_meshStreamResources->get_max_indirect_command_count();
}

VkDescriptorSet_T* GSceneRenderer2::get_bindless_set()
{
	return m_bindlessSet;
}

void GSceneRenderer2::destroy()
{	
	//X Destroy pipelines
	{
		if (m_compositionPipeline != nullptr)
		{
			m_compositionPipeline->destroy();
			delete m_compositionPipeline;
			m_compositionPipeline = nullptr;
		}
		if (m_deferredPipeline != nullptr)
		{
			m_deferredPipeline->destroy();
			delete m_deferredPipeline;
			m_deferredPipeline = nullptr;
		}
		if (m_compositionPBRPipeline != nullptr)
		{
			m_compositionPBRPipeline->destroy();
			delete m_compositionPBRPipeline;
			m_compositionPBRPipeline = nullptr;
		}
		if (m_aabbPipeline != nullptr)
		{
			m_aabbPipeline->destroy();
			delete m_aabbPipeline;
			m_aabbPipeline = nullptr;
		}
	}
	//X Destroy shader resources
	{
		if (m_compositionFragmentShaderRes != nullptr)
		{
			m_compositionFragmentShaderRes->destroy();
			delete m_compositionFragmentShaderRes;
			m_compositionFragmentShaderRes = nullptr;
		}
		if (m_compositionVertexShaderRes != nullptr)
		{
			m_compositionVertexShaderRes->destroy();
			delete m_compositionVertexShaderRes;
			m_compositionVertexShaderRes = nullptr;
		}
		if (m_deferredFragmentShaderRes != nullptr)
		{
			m_deferredFragmentShaderRes->destroy();
			delete m_deferredFragmentShaderRes;
			m_deferredFragmentShaderRes = nullptr;
		}
		if (m_deferredFragmentPBRShaderRes != nullptr)
		{
			m_deferredFragmentPBRShaderRes->destroy();
			delete m_deferredFragmentPBRShaderRes;
			m_deferredFragmentPBRShaderRes = nullptr;
		}
		if (m_deferredVertexShaderRes != nullptr)
		{
			m_deferredVertexShaderRes->destroy();
			delete m_deferredVertexShaderRes;
			m_deferredVertexShaderRes = nullptr;
		}
		if (m_cullComputeShaderRes != nullptr)
		{
			m_cullComputeShaderRes->destroy();
			delete m_cullComputeShaderRes;
			m_cullComputeShaderRes = nullptr;
		}
	}
	//X Destroy renderpass
	{
		if (m_meshStreamResources != nullptr)
		{
			m_meshStreamResources->destroy();
			delete m_meshStreamResources;
			m_meshStreamResources = nullptr;
		}
	}
	//X 
	{
		if (m_compPipeline != nullptr)
		{
			vkDestroyPipeline(p_boundedDevice->get_vk_device(), m_compPipeline, nullptr);
			m_compPipeline = nullptr;
		}
	}
}

uint32_t GSceneRenderer2::get_max_count_of_draw_data()
{
	return m_meshStreamResources->get_count_of_draw_data();
}

bool GSceneRenderer2::load_shadow_resources()
{
	VkExtent3D extent = {
		.width = uint32_t(SHADOWMAP_DIM),
		.height = uint32_t(SHADOWMAP_DIM),
		.depth = 1
	};
	uint32_t renderingFamilyIndex = p_boundedDevice->get_render_queue()->get_queue_index();
	//X Shadow  Attachment
	{
		VkImageCreateInfo inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		inf.pNext = nullptr;
		inf.flags = 0;
		inf.imageType = VK_IMAGE_TYPE_2D;
		inf.format = SHADOWMAP_FORMAT;
		inf.mipLevels = 1;
		inf.arrayLayers = 1;
		inf.samples = VK_SAMPLE_COUNT_1_BIT;
		inf.tiling = VK_IMAGE_TILING_OPTIMAL;
		inf.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		inf.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		inf.queueFamilyIndexCount = 1;
		inf.pQueueFamilyIndices = &renderingFamilyIndex;
		inf.extent = extent;
		inf.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;


		auto res = p_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);

		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}

		m_shadowAttachment = res.value();

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		range.layerCount = 1;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;
		range.levelCount = 1;

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.format = SHADOWMAP_FORMAT;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.flags = 0;
		viewInfo.subresourceRange = range;

		m_shadowAttachment->create_image_view(&viewInfo);




		VkRenderPassCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

		std::array<VkAttachmentDescription, 1> attachmentDescs;
		attachmentDescs[0] = {};
		attachmentDescs[0].format = SHADOWMAP_FORMAT;
		attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = nullptr;
		subpass.colorAttachmentCount = 0;
		subpass.pDepthStencilAttachment = &depthReference;
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = 0;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = 0;


		// Create render pass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		m_shadowPass = p_pipelineManager->create_or_get_named_renderpass("SunShadowRenderPass", &renderPassInfo);
		assert(m_shadowPass != nullptr);

		auto attachmentView = m_shadowAttachment->get_vk_image_view();

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_shadowPass->get_vk_renderpass();
		framebufferInfo.pAttachments = &attachmentView;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = SHADOWMAP_DIM;
		framebufferInfo.height = SHADOWMAP_DIM;
		framebufferInfo.layers = 1;

		assert(VK_SUCCESS == vkCreateFramebuffer(p_boundedDevice->get_vk_device(), &framebufferInfo, nullptr, &m_shadowFrameBuffer));

	}

	{
		


		//X Create Set From Layout
		auto vkLayout = m_sunShadowSetLayout->get_layout();
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_compositionPool->get_vk_descriptor_pool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &vkLayout;

		assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, &m_sunShadowSet));

	}
	{
		VkSamplerCreateInfo sampler = {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		vkCreateSampler(p_boundedDevice->get_vk_device(), &sampler, nullptr, &m_depthSampler);

	}
	//X Create Offset texture
	{
		std::vector<float> jitterData;
		generate_offsets_for_shadow_texture(JITTER_WINDOW_SIZE, JITTER_FILTER_SIZE, jitterData);
		VkImageCreateInfo inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		inf.pNext = nullptr;
		inf.flags = 0;
		inf.imageType = VK_IMAGE_TYPE_3D;
		inf.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		inf.mipLevels = 1;
		inf.arrayLayers = 1;
		inf.samples = VK_SAMPLE_COUNT_1_BIT;
		inf.tiling = VK_IMAGE_TILING_OPTIMAL;
		inf.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		inf.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		inf.queueFamilyIndexCount = 1;
		inf.pQueueFamilyIndices = &renderingFamilyIndex;
		inf.extent.width = (JITTER_FILTER_SIZE * JITTER_FILTER_SIZE)/2;
		inf.extent.height = JITTER_WINDOW_SIZE;
		inf.extent.depth = JITTER_WINDOW_SIZE;
		inf.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.layerCount = 1;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;
		range.levelCount = 1;

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		viewInfo.flags = 0;
		viewInfo.subresourceRange = range;





		auto transferOp = p_boundedDevice->get_transfer_operation();
		m_jitterImage = transferOp->init_image_to_the_gpu_from_cpu_sleep(&inf, &viewInfo, sizeof(float)* jitterData.size(), jitterData.data()).value();
	}
	return true;
}

float generate_random()
{
	static std::default_random_engine generator;
	static std::uniform_real_distribution<float> distrib(-0.5f,0.5f);
	return distrib(generator);
}
#define PI 3.141592653589793

void GSceneRenderer2::generate_offsets_for_shadow_texture(int windowSize, int filterSize, std::vector<float>& data)
{
	int bufferSize = pow(windowSize, 2) * pow(filterSize, 2) * 2;
	data.resize(bufferSize);
	int index = 0;
	for (int y = 0; y < windowSize; y++)
	{
		for (int x = 0; x < windowSize; x++)
		{
			for (int v = 0; v < filterSize; v++)
			{
				for (int u = 0; u < filterSize; u++)
				{
					float currentX = (float(u)+0.5f+ generate_random())/(filterSize);
					float currentY = (float(v) + 0.5f + generate_random()) / (filterSize);

					data[index] = sqrtf(currentY) * cosf(2 * PI * currentX);
					data[index + 1] = sqrtf(currentY) * sinf(2 * PI * currentX);

					index += 2;
				}
			}
		}
	}
}

MATERIAL_MODE GSceneRenderer2::get_current_material_mode() const noexcept
{
	return m_materialMode;
}

void GSceneRenderer2::set_material_mode(MATERIAL_MODE mode) noexcept
{
	m_materialMode = mode;
	switch (m_materialMode)
	{
	case MATERIAL_MODE_BLINN_PHONG:
		m_selectedCompositionPipeline = m_compositionPipeline;
		break;
	case MATERIAL_MODE_PBR:
		m_selectedCompositionPipeline = m_compositionPBRPipeline;
		break;
	default:
		m_selectedCompositionPipeline = m_compositionPipeline;
		break;
	}
}

IVulkanImage* GSceneRenderer2::get_sun_shadow_attachment()
{
	return m_shadowAttachment;
}

void GSceneRenderer2::fill_compute_cmd(GVulkanCommandBuffer* cmd, uint32_t frame)
{
	if (m_useMeshlet)
	{
		m_meshStreamResources->cmd_reset_indirect_buffers(cmd, frame);

		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, this->m_compMeshletPipeline);
		std::array<VkDescriptorSet, 6> descriptorSets;
		descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
		descriptorSets[1] = m_meshStreamResources->get_compute_set_by_index(frame);
		descriptorSets[2] = this->m_meshStreamResources->get_draw_set_by_index(frame);
		descriptorSets[3] = m_drawDataSet;
		descriptorSets[4] = m_cullDataSet;
		descriptorSets[5] = m_meshletStreamResources->get_set_by_index(frame);

		vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_computeMeshletPipelineLayout->get_vk_pipeline_layout(), 0, descriptorSets.size(), descriptorSets.data(), 0, 0);

		// Group counts will be increased for depth pyramit building
		vkCmdDispatch(cmd->get_handle(), 10, 1, 1);

		//X Make ready to read for indirect draw
		m_meshStreamResources->cmd_indirect_barrier_for_indirect_read(cmd, frame);

	}
	else
	{
		m_meshStreamResources->cmd_reset_indirect_buffers(cmd, frame);

		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_compPipeline);
		std::array<VkDescriptorSet, 5> descriptorSets;
		descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
		descriptorSets[1] = m_meshStreamResources->get_compute_set_by_index(frame);
		descriptorSets[2] = this->m_meshStreamResources->get_draw_set_by_index(frame);
		descriptorSets[3] = m_drawDataSet;
		descriptorSets[4] = m_cullDataSet;

		vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipelineLayout->get_vk_pipeline_layout(), 0, descriptorSets.size(), descriptorSets.data(), 0, 0);

		vkCmdDispatch(cmd->get_handle(), 10, 1, 1);

		// Group counts will be increased for depth pyramit buildin
		//X Make ready to read for indirect draw
		m_meshStreamResources->cmd_indirect_barrier_for_indirect_read(cmd, frame);
	}
}

void GSceneRenderer2::fill_aabb_cmd_for(GVulkanCommandBuffer* cmd, uint32_t frame, uint32_t nodeId)
{
	uint32_t drawId = p_sceneManager->get_draw_id_of_node(nodeId);
	if (drawId == -1 || m_useMeshlet)
		return;

	uint32_t meshIndex = m_meshStreamResources->m_globalDrawData.cpuVector[drawId].mesh;
	uint32_t transformIndex = m_meshStreamResources->m_globalDrawData.cpuVector[drawId].transformIndex;

	const auto& mesh = m_meshStreamResources->m_mergedMesh.cpuVector[meshIndex];
	auto center = glm::vec3((mesh.boundingBox.min_.x + mesh.boundingBox.max_.x)/2, (mesh.boundingBox.min_.y + mesh.boundingBox.max_.y) / 2, (mesh.boundingBox.min_.z + mesh.boundingBox.max_.z) / 2);
	auto size = (mesh.boundingBox.max_ - mesh.boundingBox.min_);
	glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);
	auto currScene = p_sceneManager->get_current_scene();
	glm::mat4 model =glm::mat4(1.f);
	uint32_t gpuTransformIndex = p_sceneManager->get_gpu_transform_index(nodeId);
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_aabbPipeline->get_vk_pipeline());
	std::array<VkDescriptorSet, 3> descriptorSets;
	descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
	descriptorSets[1] = this->m_meshStreamResources->get_draw_set_by_index(frame);
	descriptorSets[2] = m_drawDataSet;
	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_aabbDrawLayout->get_vk_pipeline_layout(), 0, descriptorSets.size(), descriptorSets.data(), 0, 0);

	AABBDrawer drawer;
	drawer.meshId = drawId;
	drawer.transform = transform;

	vkCmdPushConstants(cmd->get_handle(), m_aabbDrawLayout->get_vk_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(AABBDrawer), &drawer);

	vkCmdSetViewport(cmd->get_handle(), 0, 1, m_deferredVp->get_viewport_area());
	vkCmdSetScissor(cmd->get_handle(), 0, 1, m_deferredVp->get_scissor_area());

	vkCmdDraw(cmd->get_handle(), 36, 1, 0, 0);
}

void GSceneRenderer2::begin_and_end_fill_cmd_for_shadow(GVulkanCommandBuffer* cmd, uint32_t frame)
{
	
	auto globalData = p_sceneManager->get_global_data();
	//X Calculate LP
	auto lightPos = glm::normalize(glm::make_vec3(globalData->sunProperties.sunLightDirection))*40.f;
	//glm::mat4 depthProjectionMatrix = glm::ortho(-35.f, 15.f, -15.f, 15.f, globalData->zNear, 46.f);
	glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.f), 1.f, globalData->zNear,96.f);

	glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

	auto lp = depthProjectionMatrix* depthViewMatrix* glm::mat4(1.f);
	VkRenderPassBeginInfo inf = {};
	VkViewport port = {};
	port.maxDepth = 1.f;
	port.height = SHADOWMAP_DIM;
	port.width = SHADOWMAP_DIM;
	port.y = 0;

	VkRect2D rect = {};
	rect.offset = {};
	rect.extent.height = SHADOWMAP_DIM;
	rect.extent.width = SHADOWMAP_DIM;

	VkClearValue clearVal = {};
	clearVal.depthStencil.depth = 1.f;
	clearVal.depthStencil.stencil = 0.f;

	inf.clearValueCount = 1;
	inf.framebuffer = m_shadowFrameBuffer;
	inf.pNext = nullptr;
	inf.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	inf.renderArea = rect;
	inf.renderPass = m_shadowPass->get_vk_renderpass();
	inf.pClearValues = &clearVal;

	float depthBiasConstant = 1.25f;
	float depthBiasSlope = 1.75f;
	
	vkCmdSetDepthBias(
		cmd->get_handle(),
		depthBiasConstant,
		0.0f,
		depthBiasSlope);

	vkCmdBeginRenderPass(cmd->get_handle(), &inf, VK_SUBPASS_CONTENTS_INLINE);
	if (!m_useMeshlet)
	{
		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_sunShadowPipeline->get_vk_pipeline());
		std::array<VkDescriptorSet, 4> descriptorSets;
		descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
		descriptorSets[1] = this->m_meshStreamResources->get_draw_set_by_index(frame);
		descriptorSets[2] = m_drawDataSet;
		descriptorSets[3] = m_bindlessSet;
		vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_sunShadowLayout->get_vk_pipeline_layout(), 0, descriptorSets.size(), descriptorSets.data(), 0, 0);


		vkCmdSetViewport(cmd->get_handle(), 0, 1, &port);
		vkCmdSetScissor(cmd->get_handle(), 0, 1, &rect);

		vkCmdPushConstants(cmd->get_handle(), m_sunShadowLayout->get_vk_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &lp);

		m_meshStreamResources->bind_vertex_index_stream(cmd, frame);

		m_meshStreamResources->cmd_draw_indirect_data(cmd, frame);
	}
	else
	{
		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_sunShadowMeshPipeline->get_vk_pipeline());
		std::array<VkDescriptorSet, 5> descriptorSets;
		descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
		descriptorSets[1] = this->m_meshStreamResources->get_draw_set_by_index(frame);
		descriptorSets[2] = m_drawDataSet;
		descriptorSets[3] = m_bindlessSet;
		descriptorSets[4] = m_meshletStreamResources->get_set_by_index(frame);

		vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredletLayout->get_vk_pipeline_layout(), 0, descriptorSets.size(), descriptorSets.data(), 0, 0);

		vkCmdSetViewport(cmd->get_handle(), 0, 1, &port);
		vkCmdSetScissor(cmd->get_handle(), 0, 1, &rect);

		m_meshletStreamResources->cmd_draw_indirect_data(cmd, frame, m_meshStreamResources->m_maxIndirectDrawCommand,
			m_meshStreamResources->m_globalIndirectCommandBuffers[frame].get(), m_meshStreamResources->m_globalIndirectCommandsBeginOffset);

	}
	vkCmdEndRenderPass(cmd->get_handle());
}

const DrawData* GSceneRenderer2::get_draw_data_by_id(uint32_t drawId) const noexcept
{
	return &m_meshStreamResources->m_globalDrawData.cpuVector[drawId];
}

IGVulkanNamedRenderPass* GSceneRenderer2::get_deferred_pass() const noexcept
{
	return m_deferredPass;
}

IGVulkanNamedRenderPass* GSceneRenderer2::get_composition_pass() const noexcept
{
	return m_compositionPass;
}

std::vector<VkFormat> GSceneRenderer2::get_deferred_formats() const noexcept
{
	std::vector<VkFormat> formats(4);
	formats[0] = POSITION_TARGET_COLOR_FORMAT;
	formats[1] = ALBEDO_TARGET_COLOR_FORMAT;
	formats[2] = EMISSION_TARGET_COLOR_FORMAT;
	formats[3] = PBR_TARGET_COLOR_FORMAT;

	return formats;
}
static PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTaskExtMeth = nullptr;
void GSceneRenderer2::fill_deferred_cmd(GVulkanCommandBuffer* cmd,uint32_t frame)
{
	m_meshStreamResources->handle_gpu_datas(cmd, frame);
	if (m_useMeshlet)
	{
		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredMeshletPipeline->get_vk_pipeline());
		std::array<VkDescriptorSet, 5> descriptorSets;
		descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
		descriptorSets[1] = this->m_meshStreamResources->get_draw_set_by_index(frame);
		descriptorSets[2] = m_drawDataSet;
		descriptorSets[3] = m_bindlessSet;
		descriptorSets[4] = m_meshletStreamResources->get_set_by_index(frame);

		vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredletLayout->get_vk_pipeline_layout(), 0, descriptorSets.size(), descriptorSets.data(), 0, 0);

		vkCmdSetViewport(cmd->get_handle(), 0, 1, m_deferredVp->get_viewport_area());
		vkCmdSetScissor(cmd->get_handle(), 0, 1, m_deferredVp->get_scissor_area());
	
		m_meshletStreamResources->cmd_draw_indirect_data(cmd, frame, m_meshStreamResources->m_maxIndirectDrawCommand,
			m_meshStreamResources->m_globalIndirectCommandBuffers[frame].get(), m_meshStreamResources->m_globalIndirectCommandsBeginOffset);
	}
	else
	{
		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredPipeline->get_vk_pipeline());
		std::array<VkDescriptorSet, 4> descriptorSets;
		descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
		descriptorSets[1] = this->m_meshStreamResources->get_draw_set_by_index(frame);
		descriptorSets[2] = m_drawDataSet;
		descriptorSets[3] = m_bindlessSet;
		vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredLayout->get_vk_pipeline_layout(), 0, descriptorSets.size(), descriptorSets.data(), 0, 0);

		vkCmdSetViewport(cmd->get_handle(), 0, 1, m_deferredVp->get_viewport_area());
		vkCmdSetScissor(cmd->get_handle(), 0, 1, m_deferredVp->get_scissor_area());

		m_meshStreamResources->bind_vertex_index_stream(cmd, frame);

		m_meshStreamResources->cmd_draw_indirect_data(cmd, frame);
	}
}

void GSceneRenderer2::fill_composition_cmd(GVulkanCommandBuffer* cmd, uint32_t frame)
{
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_selectedCompositionPipeline->get_vk_pipeline());
	std::array<VkDescriptorSet_T*, 4> compositionSets;
	compositionSets[0] = m_compositionSet;
	compositionSets[1] = m_lightDataSet;
	compositionSets[2] = p_sceneManager->get_global_set_for_frame(frame);
	compositionSets[3] = m_sunShadowSet;
	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_compositionLayout->get_vk_pipeline_layout(), 0, compositionSets.size(), compositionSets.data(), 0, 0);
	auto vp = *m_compositionVp->get_viewport_area();
	vp.height *= -1;
	vp.y = 0;
	vkCmdSetViewport(cmd->get_handle(), 0, 1, &vp);
	vkCmdSetScissor(cmd->get_handle(), 0, 1, m_compositionVp->get_scissor_area());

	vkCmdDraw(cmd->get_handle(), 3, 1, 0, 0);
}

VkFormat GSceneRenderer2::get_composition_format() const noexcept
{
	return m_compositionFormat;
}

void GSceneRenderer2::set_composition_views(IVulkanImage* position, IVulkanImage* albedo, IVulkanImage* emission, IVulkanImage* pbr, VkSampler_T* sampler, IGVulkanNamedViewport* deferredVp, IGVulkanNamedViewport* compositionVp)
{
	// position // albedo // emission // pbr
	std::array<VkDescriptorImageInfo, 4> infos;
	infos[0] = { .sampler = sampler,.imageView = position->get_vk_image_view(),.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	infos[1] = { .sampler = sampler,.imageView = albedo->get_vk_image_view(),.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	infos[2] = { .sampler = sampler,.imageView = emission->get_vk_image_view(),.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	infos[3] = { .sampler = sampler,.imageView = pbr->get_vk_image_view(),.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

	std::array<VkWriteDescriptorSet, 4> writeSets;
	writeSets[0] = {};
	writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[0].descriptorCount = 1;
	writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSets[0].dstSet = m_compositionSet;
	writeSets[0].pImageInfo = &infos[0];
	writeSets[0].dstBinding = 0;

	writeSets[1] = {};
	writeSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[1].descriptorCount = 1;
	writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSets[1].dstSet = m_compositionSet;
	writeSets[1].pImageInfo = &infos[1];
	writeSets[1].dstBinding = 1;

	writeSets[2] = {};
	writeSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[2].descriptorCount = 1;
	writeSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSets[2].dstSet = m_compositionSet;
	writeSets[2].pImageInfo = &infos[2];
	writeSets[2].dstBinding = 2;

	writeSets[3] = {};
	writeSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[3].descriptorCount = 1;
	writeSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSets[3].dstSet = m_compositionSet;
	writeSets[3].pImageInfo = &infos[3];
	writeSets[3].dstBinding = 3;

	vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), writeSets.size(), writeSets.data(), 0, 0);

	m_deferredVp = deferredVp;
	m_compositionVp = compositionVp;

	{

		auto attach = m_deferredVp->get_sampler_for_named_attachment("cartcurt");
		// position // albedo // emission // pbr
		std::array<VkDescriptorImageInfo, 4> infos;
		infos[0] = { .sampler = m_depthSampler,.imageView = m_shadowAttachment->get_vk_image_view(),.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };
		infos[1] = { .sampler = attach,.imageView = m_jitterImage->get_vk_image_view(),.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		std::array<VkWriteDescriptorSet, 2> writeSets;
		writeSets[0] = {};
		writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSets[0].descriptorCount = 1;
		writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSets[0].dstSet = m_sunShadowSet;
		writeSets[0].pImageInfo = &infos[0];
		writeSets[0].dstBinding = 0;

		writeSets[1] = {};
		writeSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSets[1].descriptorCount = 1;
		writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSets[1].dstSet = m_sunShadowSet;
		writeSets[1].pImageInfo = &infos[1];
		writeSets[1].dstBinding = 1;

		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), writeSets.size(), writeSets.data(), 0, 0);
	}
}

uint32_t GSceneRenderer2::add_mesh_to_scene(const MeshData* meshData, uint32_t rendererID)
{
	return m_meshStreamResources->add_mesh_data(meshData);
}

uint32_t GSceneRenderer2::add_meshlet_to_scene(const GMeshletData* meshlet)
{
	uint32_t meshId = m_meshStreamResources->add_mesh_data(meshlet);
	uint32_t meshletId = m_meshletStreamResources->add_meshlet_data(meshlet);
	assert(meshId == meshletId);
	return meshId;
}

uint32_t GSceneRenderer2::create_draw_data(uint32_t meshIndex, uint32_t materialIndex, uint32_t transformIndex)
{
	return m_meshStreamResources->create_draw_data(meshIndex,materialIndex,transformIndex);
}
