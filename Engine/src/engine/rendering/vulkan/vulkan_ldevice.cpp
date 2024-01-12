#include "volk.h"

#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_pdevice.h"
#include "internal/engine/rendering/vulkan/vulkan_utils.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "internal/engine/manager/glogger_manager.h"
#include "engine/rendering/vulkan/ivulkan_app.h"
#include <algorithm>
#include <cassert>
#include "internal/engine/rendering/vulkan/gvulkan_buffer.h"
#include "internal/engine/rendering/vulkan/gvulkan_image.h"
#include <spdlog/fmt/fmt.h>
#include "internal/engine/rendering/vulkan/transfer/transfer_op_transfer_queue.h"
#include "internal/engine/rendering/vulkan/vma_importer.h"
#include "internal/engine/rendering/vulkan/gvulkan_pipeline_layout.h"
#include "internal/engine/rendering/vulkan/gvulkan_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "internal/engine/rendering/vulkan/gvulkan_descriptor_pool.h"
#include "internal/engine/rendering/vulkan/gvulkan_vectorized_descriptor_pool.h"
#include "engine/gengine.h"
#include "internal/engine/rendering/vulkan/vulkan_swapchain.h"
#include "internal/engine/rendering/vulkan/gvulkan_vectorized_pipeline_layout.h"
#include "internal/engine/rendering/vulkan/gvulkan_uniform_buffer.h"
#include "internal/engine/rendering/vulkan/gvulkan_vertex_state.h"
#include "internal/engine/rendering/vulkan/gvulkan_input_assembly_state.h"
#include "internal/engine/rendering/vulkan/gvulkan_rasterization_state.h"
#include "internal/engine/rendering/vulkan/gvulkan_multisample_state.h"
#include "internal/engine/rendering/vulkan/gvulkan_color_blend_state.h"
#include "internal/engine/rendering/vulkan/gvulkan_viewport_state.h"
#include "internal/engine/rendering/vulkan/gvulkan_vertex_buffer.h"
#include "internal/engine/rendering/vulkan/gvulkan_graphic_pipeline_custom_layout.h"
#include "internal/engine/rendering/vulkan/gvulkan_depth_stencil_state.h"
#include "internal/engine/rendering/vulkan/gvulkan_camera_pipeline_layout_creator.h"
#include "internal/engine/manager/gcamera_manager.h"
#include "internal/engine/rendering/vulkan/gvulkan_storage_buffer.h"
#include "internal/engine/rendering/vulkan/gvulkan_indirect_buffer.h"
#include "engine/imanager_table.h"
#include "internal/engine/manager/gscene_manager.h"

static uint32_t index = 1;

GVulkanLogicalDevice::GVulkanLogicalDevice(IGVulkanDevice* owner,GWeakPtr<IGVulkanPhysicalDevice> physicalDev, bool debugEnabled) : m_physicalDev(physicalDev),m_debugEnabled(debugEnabled)
{
	m_destroyed = true;
	m_logicalDevice = 0;
	allocator = nullptr;
	m_owner = owner;
}

GVulkanLogicalDevice::~GVulkanLogicalDevice()
{
	if (!m_destroyed)
	{
		vkDestroyDevice(m_logicalDevice, nullptr);
	}
}

bool GVulkanLogicalDevice::init()
{
	m_tryToUseMeshlet = true;

	auto physicalDevice = m_physicalDev.as_shared();
	if (!physicalDevice.is_valid())
		return false;

	m_logger = GLoggerManager::get_instance()->create_owning_glogger("GVulkanLogicalDevice");
	
	m_logger->log_d("Creating Queue Info");

	uint32_t queueIndex = physicalDevice->get_default_queue_family_index();

	QueueCreateInf queueCreateInf;

	std::vector<float> mainPriority = { 1.f };

	queueCreateInf.add_create_info(queueIndex, mainPriority);

 
	if (physicalDevice->does_support_only_transfer())
	{
		m_logger->log_d(fmt::format("Support only transfer found family index is : {}. Default Queue were : {}", physicalDevice->get_only_transfer(),queueIndex).c_str());
		queueCreateInf.add_create_info(physicalDevice->get_only_transfer(), mainPriority);
	}
	queueCreateInf.add_create_info(physicalDevice->get_compute_queue(), mainPriority);

	//X All Layers
	std::vector<VkLayerProperties> allLayerProps;
	uint32_t propCount = 0;
	
	m_logger->log_d("Enumerating Layer Properties");

	if (VK_SUCCESS != vkEnumerateDeviceLayerProperties((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), &propCount, nullptr))
		return false;
	allLayerProps.resize(propCount);
	if (VK_SUCCESS != vkEnumerateDeviceLayerProperties((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), &propCount, allLayerProps.data()))
		return false;

	if (m_debugEnabled)
	{
		m_logger->log_d("Debug enabled. Trying to find debug layer");
		for (int i = 0; i < allLayerProps.size(); i++)
		{
			if (strcmp(allLayerProps[i].layerName, VK_LAYER_NAME_DEBUG_LAYER_NAME) == 0)
			{
				m_logger->log_d("Debug layer found. Added");
				m_deviceLayers.push_back(allLayerProps[i]);
				break;
			}
		}
	}

	if (m_deviceLayers.size() == 0)
	{
		m_debugEnabled = false;
		//X TODO : LOGGER HERE FORCE CLOSING DEBUG LAYER
		m_logger->log_d("There is no debug layer. Closing debug mode by force");
	}
	m_logger->log_d("Get explicit extensions");
	std::vector<VkExtensionProperties> implicitExs;
	get_device_implicit_exs((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), implicitExs);

	//X Try to find swapchain extension
	int swapchainExsIndex = -1;
	m_logger->log_d("Trying to find swapchain extension");
	for (int i = 0; i < implicitExs.size(); i++)
	{
		if (strcmp(implicitExs[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			m_logger->log_d("Swapchain extension added");
			swapchainExsIndex = i;
		}
		else if (strcmp(implicitExs[i].extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME) == 0)
		{
			m_deviceExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(implicitExs[i]);
		}
		else if (strcmp(implicitExs[i].extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
		{
			m_deviceExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(implicitExs[i]);
			m_meshletsEnabled = true;
		}
		else if (strcmp(implicitExs[i].extensionName, VK_NV_MESH_SHADER_EXTENSION_NAME) == 0)
		{
			m_deviceExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(implicitExs[i]);
			m_meshletsEnabled = true;
		}
	}

	//X TODO : LOGGER
	if (swapchainExsIndex == -1)
		return false;

	m_logger->log_d("Swapchain extension added to the main extensions");

	//X Push swapchain extension
	m_deviceExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(implicitExs[swapchainExsIndex]);
	//X PLUGIN GOES HERE




	m_logger->log_d("Checking duplicates for layers");

	//X Check the array if plugins added same thing more than once
	
	m_deviceLayers.erase(std::unique(m_deviceLayers.begin(), m_deviceLayers.end(),
		[](const VkLayerProperties& lhs, const VkLayerProperties& rhs) {
			return std::strcmp(lhs.layerName, rhs.layerName) == 0;
		}),
		m_deviceLayers.end());


	//X Check the extension array if plugins added same thing more than once

	m_logger->log_d("Checking duplicates for extensions");

	//X Check the array if plugins added same thing more than once
	for (auto& exs : m_deviceExtensions)
	{
		exs.second.erase(std::unique(exs.second.begin(), exs.second.end(),
			[](const VkExtensionProperties& lhs, const VkExtensionProperties& rhs) {
				return std::strcmp(lhs.extensionName, rhs.extensionName) == 0;
			}),
			exs.second.end());
	}


	m_logger->log_d("Creating info layers from selected layers");

	std::vector<const char*> enabledLayers(m_deviceLayers.size());

	int vexSize = 0;
	for (const auto& exs : m_deviceExtensions)
	{
		for (const auto& ex : exs.second)
		{
			vexSize++;
		}
	}
	
	m_logger->log_d("Creating info extensions from selected extensions");

	std::vector<const char*> enabledExtensions(vexSize);

	// Make ready data for vulkan
	for (int i = 0; i <	m_deviceLayers.size(); i++)
	{
		enabledLayers[i] = m_deviceLayers[i].layerName;
	}

	int vexIndex = 0;
	for (const auto& exs :m_deviceExtensions)
	{
		for (const auto& ex : exs.second)
		{
			enabledExtensions[vexIndex] = ex.extensionName;
			vexIndex++;
		}
	}


	// NOW CREATE LOGICAL DEVICE

	m_logger->log_d("Creating device");



	VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR barycentric = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR,
		.pNext = nullptr,
		.fragmentShaderBarycentric = VK_TRUE
	};

	auto deviceFeatures = physicalDevice->get_vk_features();
	deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
	deviceFeatures.shaderInt64 = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	const VkPhysicalDeviceFeatures2 deviceFeatures2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,  .pNext = &barycentric,  .features = deviceFeatures };
	VkPhysicalDeviceVulkan11Features deviceFeatures11 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, .pNext = (void*)&deviceFeatures2 ,.shaderDrawParameters = VK_TRUE};
	VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderEXT = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,.meshShader = VK_TRUE};
	meshShaderEXT.taskShader = VK_TRUE;
	meshShaderEXT.multiviewMeshShader = VK_TRUE;
	meshShaderEXT.meshShaderQueries = VK_TRUE;
	meshShaderEXT.pNext = &deviceFeatures11;
	VkPhysicalDeviceMeshShaderFeaturesNV meshShaderNV = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV,.meshShader = VK_TRUE };
	meshShaderNV.taskShader = VK_TRUE;
	meshShaderNV.pNext = &meshShaderEXT;

	VkPhysicalDeviceVulkan12Features ftrs = {};
	ftrs.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	ftrs.scalarBlockLayout = VK_TRUE;
	ftrs.pNext = (void*)&meshShaderNV;
	ftrs.storageBuffer8BitAccess = VK_TRUE;
	ftrs.shaderInt8 = VK_TRUE;
	ftrs.descriptorIndexing = VK_TRUE;
	ftrs.descriptorBindingVariableDescriptorCount = VK_TRUE;
	ftrs.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	ftrs.runtimeDescriptorArray = VK_TRUE;
	ftrs.drawIndirectCount = VK_TRUE;
	ftrs.descriptorBindingPartiallyBound = VK_TRUE;
	ftrs.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	ftrs.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
	VkDeviceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &ftrs;
	createInfo.flags = 0;
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInf.get_queue_create_inf_count();
	createInfo.pQueueCreateInfos = queueCreateInf.data();
	createInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
	createInfo.ppEnabledLayerNames = enabledLayers.data();
	createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();
	createInfo.pEnabledFeatures = nullptr;

	if (auto res = vkCreateDevice((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), &createInfo, nullptr, &(m_logicalDevice)); res != VK_SUCCESS)
		return false;

	m_logger->log_d("Device created. Getting meta infos");

	m_defaultQueue = GVulkanQueue(this,physicalDevice->get_default_queue_family_index());

	m_computeQueue = GVulkanQueue(this,physicalDevice->get_compute_queue());

	m_computeCommandManager = GSharedPtr<GVulkanCommandBufferManager>(new GVulkanCommandBufferManager(this, &m_computeQueue, false));
	m_computeCommandManager->init();
	m_logger->log_d("Checking support only transfer");

	if (physicalDevice->does_support_only_transfer())
	{
		m_logger->log_d("Transfer queue found creating transfer queue");

		m_transferQueue = GVulkanQueue(this, physicalDevice->get_only_transfer());
		m_logger->log_d(fmt::format("All bits for transfer queue : {}",m_transferQueue.get_all_supported_operations_as_string()).c_str());

		uint32_t transferCommandCount = 10;
		m_logger->log_d(fmt::format("Creating transfer commands. Count is : {}", std::to_string(transferCommandCount)).c_str());

		// 10 is okey for that moment
		//X TODO : PARAMETERIZE THIS
		m_transferOps = std::unique_ptr<ITransferOperations>(new TransferOpTransferQueue(this,10));

		if (!m_transferOps->init())
		{
			return false;
		}
	}

	else
	{
		m_logger->log_e("Application doesn't support gpu without transfer optimization");
		// Doesn't support transfer queue.
		assert(false);
	}
	if (!m_defaultQueue.is_valid())
	{
		return false;
	}

	bool ok = create_vma_allocator();

	if (!ok)
	{
		m_logger->log_d("Couldn't create vma allocator. Logical device initialization failed");
		return false;
	}
	m_inited = true;
	m_destroyed = false;

	s_instance = this;

	return true;
}

bool GVulkanLogicalDevice::is_valid() const
{
	return m_inited && !m_destroyed;
}

void GVulkanLogicalDevice::destroy()
{
	m_transferOps->destroy();
	m_computeCommandManager->destroy();
	vmaDestroyAllocator(allocator);

	vkDestroyDevice(m_logicalDevice, nullptr);
	m_destroyed = true;
	s_instance = nullptr;
}

VkDevice_T* GVulkanLogicalDevice::get_vk_device()
{
	return m_logicalDevice;
}

IGVulkanPhysicalDevice* GVulkanLogicalDevice::get_bounded_physical_device()
{
	auto physical=  m_physicalDev.as_shared();
	return physical.get();
}
IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_custom_color_blend_state(const VkPipelineColorBlendAttachmentState* attachment, const VkPipelineColorBlendStateCreateInfo* inf)
{
	return new GVulkanColorBlendState(attachment, inf);
}
IGVulkanQueue* GVulkanLogicalDevice::get_present_queue() noexcept
{
	return &m_defaultQueue;
}

IGVulkanQueue* GVulkanLogicalDevice::get_render_queue() noexcept
{
	return &m_defaultQueue;
}

IGVulkanQueue* GVulkanLogicalDevice::get_resource_queue() noexcept
{
	if(!m_transferQueue.is_valid())
		return &m_defaultQueue;
	return &m_transferQueue;
}

bool GVulkanLogicalDevice::begin_command_buffer_record(GVulkanCommandBuffer* buff)
{
	//X TODO : BUFF RETURN BOOL
	if (buff != nullptr)
		buff->begin();
	else
		return false;
	return true;
}

void GVulkanLogicalDevice::end_command_buffer_record(GVulkanCommandBuffer* buff)
{
	buff->end();
}

std::expected<IVulkanBuffer*, VULKAN_BUFFER_CREATION_ERROR> GVulkanLogicalDevice::create_buffer(uint64_t size, uint32_t bufferUsageFlag, VmaMemoryUsage memoryUsageFlag)
{
	//X TODO : GDNEWDA
	GVulkanBuffer* buff = new GVulkanBuffer(this, allocator,size);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = bufferUsageFlag;
	auto indexData = index++;
	VmaAllocationCreateInfo allocInfo = {};
	if (memoryUsageFlag == 0)
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	else
		allocInfo.usage = memoryUsageFlag;

	VkBuffer buffer;
	VmaAllocation allocation;
	
	if (VK_SUCCESS == vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr))
	{
		buff->m_inited = true;
		buff->m_allocationBlock = allocation;
		buff->m_buffer = buffer;
		vmaSetAllocationName(allocator, allocation, fmt::format("BUFFER_{}", indexData).c_str());
		return buff;
	}
	else
	{
		delete buff;
		return std::unexpected(VULKAN_BUFFER_CREATION_ERROR_UNKNOWN);
	}
}

std::expected<IVulkanImage*, VULKAN_IMAGE_CREATION_ERROR> GVulkanLogicalDevice::create_image(const VkImageCreateInfo* imageCreateInfo, VmaMemoryUsage memoryUsageFlag)
{
	GVulkanImage* image = new GVulkanImage(this, allocator);
	auto indexData = index++;

	VmaAllocationCreateInfo dimg_allocinfo = {};
	if (memoryUsageFlag == 0)
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_AUTO;
	else
		dimg_allocinfo.usage = memoryUsageFlag;
	
	VkImage vkImage;
	VmaAllocation allocation;
	
	auto code = vmaCreateImage(allocator, imageCreateInfo, &dimg_allocinfo, &vkImage, &allocation, nullptr);
	
	if (VK_SUCCESS == code)
	{
		image->m_inited = true;
		image->m_allocationBlock = allocation;
		image->m_image = vkImage;
		image->m_creationInfo = *imageCreateInfo;
		vmaSetAllocationName(allocator,allocation,fmt::format("BUFFER_{}",indexData).c_str());
		return image;
	}
	else
	{
		delete image;
		return std::unexpected(VULKAN_IMAGE_CREATION_ERROR_UNKNOWN);

	}
}

IGVulkanDevice* GVulkanLogicalDevice::get_owner() noexcept
{
	return m_owner;
}

ITransferOperations* GVulkanLogicalDevice::get_transfer_operation()
{
	return m_transferOps.get();
}

std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> GVulkanLogicalDevice::get_wait_and_begin_transfer_cmd()
{
	return m_transferOps->get_wait_and_begin_transfer_cmd();
}

std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> GVulkanLogicalDevice::get_wait_and_begin_transfer_cmd(uint64_t timeout)
{
	return m_transferOps->get_wait_and_begin_transfer_cmd(timeout);
}

void GVulkanLogicalDevice::finish_execute_and_wait_transfer_cmd(ITransferHandle* handle)
{
	m_transferOps->finish_execute_and_wait_transfer_cmd(handle);
}

IGVulkanPipelineLayout* GVulkanLogicalDevice::create_and_init_pipeline_layout(VkDescriptorSetLayout_T* layout)
{
	GVulkanPipelineLayout* glayout = new GVulkanPipelineLayout(this, layout);
	auto res = glayout->init();
	if (!res)
	{
		delete glayout;
		return nullptr;
	}
	return glayout;
}

IGVulkanGraphicPipeline* GVulkanLogicalDevice::create_and_init_default_graphic_pipeline_for_vp(IGVulkanViewport* vp,const std::vector< IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states)
{
	GVulkanGraphicPipeline* pipeline = new GVulkanGraphicPipeline(this, vp->get_render_pass(), shaderStages, states,0);
	auto res = pipeline->init();
	if (!res)
	{
		pipeline->destroy();
		delete pipeline;
		return nullptr;
	}
	return pipeline;
}

IGVulkanDescriptorPool* GVulkanLogicalDevice::create_and_init_default_pool(uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount)
{
	GVulkanDescriptorPool* pool = new GVulkanDescriptorPool(this,GEngine::get_instance()->get_swapchain()->get_total_image(),uniformBufferCount,storageBufferCount,samplerCount);
	auto res = pool->init();
	if (!res)
	{
		delete pool;
		return nullptr;
	}
	return pool;
}

IGVulkanDescriptorPool* GVulkanLogicalDevice::create_and_init_vector_pool(const std::unordered_map<VkDescriptorType, int>& typeMap)
{
	GVulkanVectorizedDescriptorPool* pool = new GVulkanVectorizedDescriptorPool(this, GEngine::get_instance()->get_swapchain()->get_total_image(), typeMap);
	auto res = pool->init();
	if (!res)
	{
		delete pool;
		return nullptr;
	}
	return pool;
}

IGVulkanPipelineLayout* GVulkanLogicalDevice::create_and_init_vector_pipeline_layout(const std::vector<VkDescriptorSetLayout_T*>& layouts)
{
	GVulkanVectorizedPipelineLayout* layout = new GVulkanVectorizedPipelineLayout(this, layouts);
	bool res = layout->init();

	if (!res)
	{
		delete layout;
		return nullptr;
	}
	return layout;
}

IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_vertex_input_state(const std::vector<VkVertexInputBindingDescription>* vertexBindingDescription, const std::vector<VkVertexInputAttributeDescription>* attributeDescription)
{	
	if (vertexBindingDescription == nullptr && attributeDescription == nullptr)
	{
		return new GVulkanVertexState();
	}
	
	return new GVulkanVertexState(vertexBindingDescription == nullptr ? std::vector<VkVertexInputBindingDescription>() : *vertexBindingDescription, attributeDescription == nullptr ? std::vector<VkVertexInputAttributeDescription>() : *attributeDescription);

}
IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_custom_depth_stencil_state(const VkPipelineDepthStencilStateCreateInfo* info)
{
	return new GVulkanDepthStencilState(info);
}
IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_default_input_assembly_state()
{
	return new GVulkanInputAssemblyState();
}

IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_input_assembly_state(VkPrimitiveTopology topology, bool resetAfterIndexedDraw)
{
	return new GVulkanInputAssemblyState(topology,resetAfterIndexedDraw);
}

IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_default_rasterization_state()
{
	return new GVulkanRasterizationState();
}

IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_default_none_multisample_state()
{
	return new GVulkanMultiSampleState();
}

IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_default_color_blend_state()
{
	return new GVulkanColorBlendState();
}

IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_default_viewport_state(uint32_t width, uint32_t height)
{
	return new GVulkanViewportState(width,height);
}

GVulkanLogicalDevice* GVulkanLogicalDevice::get_instance()
{
	assert(s_instance != nullptr);
	return s_instance;
}

bool GVulkanLogicalDevice::create_vma_allocator()
{
	m_logger->log_d("Creating VMA Allocator");

	auto physicalDev = m_physicalDev.as_shared();

	IGVulkanApp* app = physicalDev->get_bounded_app();

	assert(app != nullptr);

	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
	vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
	vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
	vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
	vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
	vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
	vulkanFunctions.vkCreateImage = vkCreateImage;
	vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
	vulkanFunctions.vkDestroyImage = vkDestroyImage;
	vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vulkanFunctions.vkFreeMemory = vkFreeMemory;
	vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
	vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
	vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
	vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
	vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vulkanFunctions.vkMapMemory = vkMapMemory;
	vulkanFunctions.vkUnmapMemory = vkUnmapMemory;


	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
	allocatorCreateInfo.physicalDevice = (VkPhysicalDevice)physicalDev->get_vk_physical_device();
	allocatorCreateInfo.device = m_logicalDevice;
	allocatorCreateInfo.instance = (VkInstance)app->get_vk_instance();
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

	return VK_SUCCESS == vmaCreateAllocator(&allocatorCreateInfo, &allocator);

}

IGVulkanQueue* GVulkanLogicalDevice::get_compute_queue() noexcept
{
	return &m_computeQueue;
}

GVulkanCommandBuffer* GVulkanLogicalDevice::create_compute_command_buffer()
{
	return m_computeCommandManager->create_buffer(true);
}

std::expected<IVulkanBuffer*, VULKAN_BUFFER_CREATION_ERROR> GVulkanLogicalDevice::create_shared_buffer(uint32_t size, uint32_t bufferUsageFlags, VmaMemoryUsage memoryUsageFlag, const std::vector<IGVulkanQueue*>* queues)
{
	GVulkanBuffer* buff = new GVulkanBuffer(this, allocator, size);

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = bufferUsageFlags;
	bufferInfo.queueFamilyIndexCount = queues->size();

	std::vector<uint32_t> vkqueues;
	for (auto vq : *queues)
	{
		vkqueues.push_back(vq->get_queue_index());
	}
	bufferInfo.pQueueFamilyIndices = vkqueues.data();
	bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	auto indexData = index++;
	VmaAllocationCreateInfo allocInfo = {};
	if (memoryUsageFlag == 0)
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	else
		allocInfo.usage = memoryUsageFlag;

	VkBuffer buffer;
	VmaAllocation allocation;

	if (VK_SUCCESS == vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr))
	{
		buff->m_inited = true;
		buff->m_allocationBlock = allocation;
		buff->m_buffer = buffer;
		vmaSetAllocationName(allocator, allocation, fmt::format("BUFFER_{}", indexData).c_str());
		return buff;
	}
	else
	{
		delete buff;
		return std::unexpected(VULKAN_BUFFER_CREATION_ERROR_UNKNOWN);
	}
}

IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_custom_rasterization_state(const VkPipelineRasterizationStateCreateInfo* state)
{
	return new GVulkanRasterizationState(state);
}

IGVulkanGraphicPipeline* GVulkanLogicalDevice::create_and_init_graphic_pipeline_injector_for_renderpass(IGVulkanRenderPass* vp, const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states, uint32_t framesInFlight, IGVulkanGraphicPipelineLayoutCreator* injector)
{
	GVulkanGraphicPipelineCustomLayout* cpipe = new GVulkanGraphicPipelineCustomLayout(this, vp, shaderStages, states, injector, 0);
	bool inited = cpipe->init();
	if (!inited)
	{
		cpipe->destroy();
		delete cpipe;
		return nullptr;
	}

	return cpipe;
}

bool GVulkanLogicalDevice::has_meshlet_support() const noexcept
{
	return m_meshletsEnabled;
}

bool GVulkanLogicalDevice::use_meshlet() const noexcept
{
	return has_meshlet_support() && m_tryToUseMeshlet;
}


std::expected<IGVulkanIndirectBuffer*, VULKAN_BUFFER_CREATION_ERROR> GVulkanLogicalDevice::create_indirect_buffer(uint32_t size)
{
	//X For debugging purposes cpu only
	auto res = create_buffer(size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (!res.has_value())
	{
		return std::unexpected(res.error());
	}
	GVulkanIndirectBuffer* buff = new GVulkanIndirectBuffer(res.value());
	return buff;
}

std::expected<IGVulkanStorageBuffer*, VULKAN_BUFFER_CREATION_ERROR> GVulkanLogicalDevice::create_storage_buffer(uint32_t size)
{
	//X TODO : For debugging purposes cpu only
	auto res = create_buffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (!res.has_value())
	{
		return std::unexpected(res.error());
	}
	GVulkanStorageBuffer* buff = new GVulkanStorageBuffer(res.value());
	return buff;
}


IGVulkanDescriptorPool* GVulkanLogicalDevice::create_and_init_vector_pool(const std::unordered_map<VkDescriptorType, int>& typeMap, uint32_t frameInFlight)
{
	GVulkanVectorizedDescriptorPool* pool = new GVulkanVectorizedDescriptorPool(this, frameInFlight, typeMap);
	auto res = pool->init();
	if (!res)
	{
		delete pool;
		return nullptr;
	}
	return pool;
}

IGVulkanGraphicPipeline* GVulkanLogicalDevice::create_and_init_graphic_pipeline_injector_for_vp(IGVulkanViewport* vp, const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states, IGVulkanGraphicPipelineLayoutCreator* injector)
{
	GVulkanGraphicPipelineCustomLayout* cpipe = new GVulkanGraphicPipelineCustomLayout(this,vp->get_render_pass(),shaderStages,states,injector,0);
	bool inited = cpipe->init();
	if (!inited)
	{
		cpipe->destroy();
		delete cpipe;
		return nullptr;
	}

	return cpipe;
}

std::expected<IGVulkanVertexBuffer*, VULKAN_BUFFER_CREATION_ERROR> GVulkanLogicalDevice::create_vertex_buffer(uint64_t size)
{
	auto res = create_buffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (!res.has_value())
	{
		return std::unexpected(res.error());
	}
	GVulkanVertexBuffer* buff = new GVulkanVertexBuffer(res.value());
	return buff;
}
IGVulkanGraphicPipeline* GVulkanLogicalDevice::create_and_init_default_graphic_pipeline_injector_for_vp(IGVulkanViewport* vp, const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states, uint32_t framesInFlight)
{
	auto scene = ((GSharedPtr<IGSceneManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();
	auto injector = new GVulkanCameraLayoutCreator(this, scene, framesInFlight);
	GVulkanGraphicPipelineCustomLayout* cpipe = new GVulkanGraphicPipelineCustomLayout(this, vp->get_render_pass(), shaderStages, states, injector, 0);
	bool inited = cpipe->init();
	if (!inited)
	{
		cpipe->destroy();
		delete cpipe;
		return nullptr;
	}

	return cpipe;
}
IGVulkanGraphicPipelineState* GVulkanLogicalDevice::create_default_depth_stencil_state()
{
	return new GVulkanDepthStencilState();
}
std::expected<IGVulkanUniformBuffer*, VULKAN_BUFFER_CREATION_ERROR> GVulkanLogicalDevice::create_uniform_buffer(uint32_t size)
{
	
	auto res = create_buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (!res.has_value())
	{
		return std::unexpected(res.error());
	}
	
	GVulkanUniformBuffer* buff = new GVulkanUniformBuffer(res.value());
	return buff;
}
