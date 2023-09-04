#ifndef GVULKAN_LDEVICE_H
#define GVULKAN_LDEVICE_H

#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_queue.h"

#include "public/core/templates/shared_ptr.h"
#include "engine/io/iowning_glogger.h"

#include <unordered_map>
#include <string>
#include <vma/vk_mem_alloc.h>
#include <memory>


class ITransferOperations;
class IGVulkanPhysicalDevice;
class GVulkanCommandBufferManager;
class IGVulkanPipelineLayout;
struct VkDescriptorLayout_T;
class IGVulkanGraphicPipeline;
class IVulkanShaderStage;
class IGVulkanDescriptorPool;


class GVulkanLogicalDevice : public IGVulkanLogicalDevice
{
public:
	GVulkanLogicalDevice(IGVulkanDevice* owner,GWeakPtr<IGVulkanPhysicalDevice> physicalDev, bool debugEnabled = true);

	~GVulkanLogicalDevice();

	virtual bool init() override;

	virtual bool is_valid() const override;

	virtual void destroy() override;

	virtual VkDevice_T* get_vk_device() override;

	virtual IGVulkanPhysicalDevice* get_bounded_physical_device() override;

	//X TODO : MAYBE THIS NEEDS TO BE EXPORTED 
	inline GVulkanQueue* get_queue()
	{
		return &m_defaultQueue;
	}

	virtual IGVulkanQueue* get_present_queue() noexcept override;

	virtual IGVulkanQueue* get_render_queue() noexcept override;

	virtual IGVulkanQueue* get_resource_queue() noexcept override;

	virtual bool begin_command_buffer_record(GVulkanCommandBuffer* buff) override;

	virtual void end_command_buffer_record(GVulkanCommandBuffer* buff) override;

	virtual std::expected<IVulkanBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_buffer(uint64_t size, uint32_t bufferUsageFlag, VmaMemoryUsage memoryUsageFlag ) override;

	virtual std::expected< IVulkanImage*, VULKAN_IMAGE_CREATION_ERROR> create_image(const VkImageCreateInfo* imageCreateInfo, VmaMemoryUsage memoryUsageFlag) override;

	virtual IGVulkanDevice* get_owner() noexcept override;

	virtual ITransferOperations* get_transfer_operation() override;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd() override;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd(uint64_t timeout) override;

	virtual void finish_execute_and_wait_transfer_cmd(ITransferHandle* handle) override;
	
	virtual std::expected<IGVulkanUniformBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_uniform_buffer(uint32_t size) override;
	
	virtual IGVulkanPipelineLayout* create_and_init_pipeline_layout(VkDescriptorSetLayout_T* layout);
	
	virtual IGVulkanGraphicPipeline* create_and_init_default_graphic_pipeline_for_vp(IGVulkanViewport* vp,
		const std::vector< IVulkanShaderStage*>& shaderStages,const std::vector<IGVulkanGraphicPipelineState*>& states) override;

	virtual IGVulkanDescriptorPool* create_and_init_default_pool(uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount) override;

	virtual IGVulkanDescriptorPool* create_and_init_vector_pool(const std::unordered_map<VkDescriptorType, int>& typeMap) override;

	virtual IGVulkanPipelineLayout* create_and_init_vector_pipeline_layout(const std::vector<VkDescriptorSetLayout_T*>& layouts) override;

	virtual IGVulkanGraphicPipelineState* create_vertex_input_state(const std::vector< VkVertexInputBindingDescription>* vertexBindingDescription, 
		const std::vector< VkVertexInputAttributeDescription>* attributeDescription) override;

	virtual IGVulkanGraphicPipelineState* create_default_input_assembly_state() override;

	virtual IGVulkanGraphicPipelineState* create_input_assembly_state(VkPrimitiveTopology topology,bool resetAfterIndexedDraw) override;

	virtual IGVulkanGraphicPipelineState* create_default_rasterization_state() override;

	virtual IGVulkanGraphicPipelineState* create_default_none_multisample_state() override;

	virtual IGVulkanGraphicPipelineState* create_default_color_blend_state() override;
	virtual IGVulkanGraphicPipelineState* create_default_viewport_state(uint32_t width,uint32_t height) override;

	virtual std::expected<IGVulkanVertexBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_vertex_buffer(uint64_t size) override;

	virtual IGVulkanGraphicPipeline* create_and_init_graphic_pipeline_injector_for_vp(IGVulkanViewport* vp, const std::vector<IVulkanShaderStage*>& shaderStages,
		const std::vector<IGVulkanGraphicPipelineState*>& states, IGVulkanGraphicPipelineLayoutCreator* injector) override;

	virtual IGVulkanGraphicPipelineState* create_default_depth_stencil_state() override;

	static GVulkanLogicalDevice* get_instance();
private:
	bool create_vma_allocator();

private:

	inline static GVulkanLogicalDevice* s_instance;

	std::unique_ptr< ITransferOperations> m_transferOps;

	bool m_destroyed;
	bool m_inited = false;
	bool m_debugEnabled;
	GWeakPtr<IGVulkanPhysicalDevice> m_physicalDev;
	GSharedPtr<GVulkanCommandBufferManager> m_defaultCommandManager;
	GSharedPtr<IOwningGLogger> m_logger;

	GVulkanQueue m_defaultQueue;
	GVulkanQueue m_transferQueue;

	IGVulkanDevice* m_owner;

	std::vector<VkLayerProperties> m_deviceLayers;
	std::unordered_map<std::string, std::vector<VkExtensionProperties>> m_deviceExtensions;

	VkDevice m_logicalDevice;
	VmaAllocator allocator;


};
#endif // GVULKAN_LDEVICE_H