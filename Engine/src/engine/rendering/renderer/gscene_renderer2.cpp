#include "volk.h"
#include "internal/engine/rendering/renderer/gscene_renderer2.h"
#include <array>
#include <vma/vk_mem_alloc.h>
#include <unordered_map>
#include "engine/resource/igshader_resource.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "engine/rendering/vulkan/named/igvulkan_named_sampler.h"
#include "engine/rendering/vulkan/named/igvulkan_named_graphic_pipeline.h"


#define POSITION_TARGET_COLOR_FORMAT VK_FORMAT_R32G32B32A32_SFLOAT
#define EMISSION_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define ALBEDO_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define PBR_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define DEPTH_TARGET_COLOR_FORMAT VK_FORMAT_D32_SFLOAT

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
}

bool GSceneRenderer2::init(VkDescriptorSetLayout_T* globalUniformSet)
{
	m_meshStreamResources = new GPUMeshStreamResources(p_boundedDevice, 7, m_framesInFlight, p_pipelineManager);
	assert(m_meshStreamResources->init(calculate_nearest_10mb<float>(), calculate_nearest_10mb<uint32_t>(), calculate_nearest_1mb<GMeshData>(),
		calculate_nearest_1mb<DrawData>()));
	m_globalDrawCullBuffer.reset(p_boundedDevice->create_buffer(sizeof(DrawCullData),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_globalDrawCullBufferMappedMem = m_globalDrawCullBuffer->map_memory();

	

	//X Find global transform data size
	uint32_t countOfTranssformMatrix = m_meshStreamResources->get_count_of_draw_data();

	m_globalTransformData.gpuBuffer.reset(p_boundedDevice->create_buffer(countOfTranssformMatrix * sizeof(glm::mat4), 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_globalTransformData.create_internals();

	m_globalMaterialData.gpuBuffer.reset(p_boundedDevice->create_buffer(calculate_nearest_10mb<MaterialDescription>() * sizeof(MaterialDescription),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_globalMaterialData.create_internals();
	//X Create DrawDataSet
	{
		{
			//X Transform Buffer
			std::array<VkDescriptorSetLayoutBinding, 2> bindings;
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[0].pImmutableSamplers = nullptr;

			//X Material Buffer
			bindings[1].binding = 1;
			bindings[1].descriptorCount = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[1].pImmutableSamplers = nullptr;


			VkDescriptorSetLayoutCreateInfo setinfo = {};
			setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setinfo.pNext = nullptr;
			setinfo.bindingCount = bindings.size();
			setinfo.flags = 0;
			setinfo.pBindings = bindings.data();

			m_drawDataSetLayout = p_pipelineManager->create_or_get_named_set_layout("DrawDataSetLayout", &setinfo);
		}
		//X Cull Data Layout
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[0].stageFlags =  VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[0].pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo setinfo = {};
			setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setinfo.pNext = nullptr;
			setinfo.bindingCount = bindings.size();
			setinfo.flags = 0;
			setinfo.pBindings = bindings.data();

			m_cullDataLayout = p_pipelineManager->create_or_get_named_set_layout("CullDataSetLayout", &setinfo);
		}
		//X Bindless Texture 
		{
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
			pool_info.maxSets = MAX_BINDLESS_TEXTURE * m_framesInFlight;
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
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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

			assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &alloc_info, &m_bindlessSet));
		}
		//X Composition layout
		{
			//X First create pool
			{
				//X First create necessary pool
				std::unordered_map<VkDescriptorType, int> map;
				map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4);
				map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
				map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);

				//X We allocate three set
				m_compositionPool = p_boundedDevice->create_and_init_vector_pool(map, 3);

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
			
				//X Create drawdataset 
				auto vkSetLayout = m_drawDataSetLayout->get_layout();
				allocInfo.pSetLayouts = &vkSetLayout;

				assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, &m_drawDataSet));
				vkSetLayout = m_cullDataLayout->get_layout();

				assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, &m_cullDataSet));
				
				//X Write Sets
				{
					//X First Draw Data Set
					std::array<VkDescriptorBufferInfo, 2> infos;
					infos[0] = { .buffer = m_globalTransformData.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalTransformData.gpuBuffer->get_size() };
					infos[1] = { .buffer = m_globalMaterialData.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalMaterialData.gpuBuffer->get_size() };

					std::array<VkWriteDescriptorSet, 2> writeSets;
					writeSets[0] = {};
					writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSets[0].descriptorCount = 1;
					writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					writeSets[0].dstBinding = 0;
					writeSets[0].dstSet = m_drawDataSet;
					writeSets[0].pBufferInfo = &infos[0];

					writeSets[1] = {};
					writeSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSets[1].descriptorCount = 1;
					writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					writeSets[1].dstBinding = 1;
					writeSets[1].dstSet = m_drawDataSet;
					writeSets[1].pBufferInfo = &infos[1];

					vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), writeSets.size(), writeSets.data(), 0, 0);


					//X Now Write Cull data
					infos[0] = { .buffer = m_globalDrawCullBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalDrawCullBuffer->get_size() };
					writeSets[0].dstSet = m_cullDataSet;
					writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

					vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), 1, writeSets.data(), 0, 0);

				}
			}

		}
	}

	//X Create pipeline layouts
	{
		//X Deferred Layout
		{
			std::array<VkDescriptorSetLayout, 4> setLayouts;
			setLayouts[0] = globalUniformSet;
			setLayouts[1] = m_meshStreamResources->get_draw_set_layout()->get_layout();
			setLayouts[2] = m_drawDataSetLayout->get_layout();
			setLayouts[3] = m_bindlessSetLayout;

			VkPipelineLayoutCreateInfo inf = {};
			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 0;
			inf.pPushConstantRanges = nullptr;

			m_deferredLayout = p_pipelineManager->create_or_get_named_pipeline_layout("DeferredPipelineLayout",&inf);
			assert(m_deferredLayout != nullptr);
		}

		//X Composition layout
		{
			std::array<VkDescriptorSetLayout, 1> setLayouts;
			setLayouts[0] = m_compositionSetLayout;
			VkPipelineLayoutCreateInfo inf = {};
			inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			inf.flags = 0;
			inf.setLayoutCount = setLayouts.size();
			inf.pSetLayouts = setLayouts.data();
			inf.pushConstantRangeCount = 0;
			inf.pPushConstantRanges = nullptr;

			m_compositionLayout = p_pipelineManager->create_or_get_named_pipeline_layout("composition_layout",&inf);
			assert(m_compositionLayout != nullptr);
		}
		//X Compute Layout
		{
			std::array<VkDescriptorSetLayout, 5> setLayouts;
			setLayouts[0] = globalUniformSet;
			setLayouts[1] = m_meshStreamResources->get_indirect_set_layout()->get_layout();
			setLayouts[2] = m_meshStreamResources->get_draw_set_layout()->get_layout();
			setLayouts[3] = m_drawDataSetLayout->get_layout();
			setLayouts[4] = m_cullDataLayout->get_layout();

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
		m_compositionVertexShaderRes = p_resourceManager->create_shader_resource("compositionVertex", "SceneRenderer", "./data/shader/composition.glsl_vert").value();
		m_compositionFragmentShaderRes = p_resourceManager->create_shader_resource("compositionFrag", "SceneRenderer", "./data/shader/composition.glsl_frag").value();
		m_cullComputeShaderRes = p_resourceManager->create_shader_resource("cullCompute", "SceneRenderer", "./data/shader/cull.glsl_comp").value();

		assert(m_deferredVertexShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_deferredFragmentShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_compositionVertexShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_compositionFragmentShaderRes->load() == RESOURCE_INIT_CODE_OK);
		assert(m_cullComputeShaderRes->load() == RESOURCE_INIT_CODE_OK);

	}

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


			states.push_back(p_boundedDevice->create_vertex_input_state(&bindingDescriptions, &attributeDescriptions));
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

			stages[0] = p_shaderManager->create_shader_stage_from_shader_res(m_compositionVertexShaderRes).value();
			stages[1] = p_shaderManager->create_shader_stage_from_shader_res(m_compositionFragmentShaderRes).value();
			delete states[3];
			delete states[5];
			states[3] = p_boundedDevice->create_default_color_blend_state();
			states[5] = p_boundedDevice->create_custom_depth_stencil_state(&dsInfo);
			m_compositionPipeline = new GVulkanNamedGraphicPipeline(p_boundedDevice, m_compositionPass, "composition_pipeline");
			m_compositionPipeline->init(m_compositionLayout, stages, states);

			delete stages[0];
			delete stages[1];

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
		}

	}
	return true;
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
		m_globalTransformData.destroy();
		m_globalMaterialData.destroy();
	}
	//X 
	{
		if (m_compPipeline != nullptr)
		{
			vkDestroyPipeline(p_boundedDevice->get_vk_device(), m_compPipeline, nullptr);
			m_compPipeline = nullptr;
		}
		if (m_globalDrawCullBuffer.get() != nullptr)
		{
			m_globalDrawCullBuffer->unload();
			m_globalDrawCullBuffer.reset();
		}
	}
}

void GSceneRenderer2::fill_compute_cmd(GVulkanCommandBuffer* cmd, uint32_t frame)
{
	m_meshStreamResources->cmd_reset_indirect_buffers(cmd, frame);

	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE,m_compPipeline);
	std::array<VkDescriptorSet, 5> descriptorSets;
	descriptorSets[0] = p_sceneManager->get_global_set_for_frame(frame);
	descriptorSets[1] = m_meshStreamResources->get_compute_set_by_index(frame);
	descriptorSets[2] = this->m_meshStreamResources->get_draw_set_by_index(frame);
	descriptorSets[3] = m_drawDataSet;
	descriptorSets[4] = m_cullDataSet;

	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipelineLayout->get_vk_pipeline_layout(),0, descriptorSets.size(), descriptorSets.data(),0,0);
	
	// Group counts will be increased for depth pyramit building
	vkCmdDispatch(cmd->get_handle(), 10, 1, 1);

	//X Make ready to read for indirect draw
	m_meshStreamResources->cmd_indirect_barrier_for_indirect_read(cmd, frame);
}

void GSceneRenderer2::update_cull_data(DrawCullData& cullData)
{
	cullData.drawCount = m_meshStreamResources->get_max_indirect_command_count();
	memcpy(m_globalDrawCullBufferMappedMem, &cullData, sizeof(DrawCullData));
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

void GSceneRenderer2::fill_deferred_cmd(GVulkanCommandBuffer* cmd,uint32_t frame)
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

	m_meshStreamResources->bind_vertex_index_stream(cmd,frame);

	m_meshStreamResources->cmd_draw_indirect_data(cmd,frame);
}

void GSceneRenderer2::fill_composition_cmd(GVulkanCommandBuffer* cmd, uint32_t frame)
{
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_compositionPipeline->get_vk_pipeline());
	auto vkDescriptorSet = m_compositionSet;
	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_compositionLayout->get_vk_pipeline_layout(), 0, 1, &vkDescriptorSet, 0, 0);

	vkCmdSetViewport(cmd->get_handle(), 0, 1, m_compositionVp->get_viewport_area());
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
}

uint32_t GSceneRenderer2::add_material_to_scene(const std::vector<MaterialDescription>& desc)
{
	return m_globalMaterialData.add_to_buffer(desc);
}

uint32_t GSceneRenderer2::add_mesh_to_scene(const MeshData* meshData, uint32_t rendererID)
{
	return m_meshStreamResources->add_mesh_data(meshData);
}

uint32_t GSceneRenderer2::create_draw_data(uint32_t meshIndex, uint32_t materialIndex, uint32_t transformIndex)
{
	return m_meshStreamResources->create_draw_data(meshIndex,materialIndex,transformIndex);
}

uint32_t GSceneRenderer2::add_default_transform()
{
	std::vector<glm::mat4> transforms(1);
	transforms[0] = glm::identity<glm::mat4>();
	return m_globalTransformData.add_to_buffer(transforms);
}

void GSceneRenderer2::set_transform_by_index(uint32_t index, glm::mat4* data)
{
	assert(index < (m_globalTransformData.gpuBuffer->get_size() / sizeof(float)*16));
	memcpy(data,&m_globalTransformData.gpuCurrentPos[index],sizeof(float)*16);
}
