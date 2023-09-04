#ifndef IVULKAN_L_DEVICE_H
#define IVULKAN_L_DEVICE_H

#include "engine/GEngine_EXPORT.h"
#include <expected>
#include <cstdint>
#include <unordered_map>

class IGVulkanPhysicalDevice;
class IGVulkanQueue;
class GVulkanCommandBuffer;
class IVulkanBuffer;
class IVulkanImage;
class IGVulkanDevice;
class IGVulkanPipelineLayout;
class IGVulkanGraphicPipeline;
class IGVulkanVertexBuffer;
class IGVulkanGraphicPipelineLayoutCreator;

struct VkImageCreateInfo;
enum VkDescriptorType;
enum VkPrimitiveTopology;
struct VkDescriptorSetLayout_T;
struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;
class IVulkanShaderStage;

enum VkFormat;

enum VULKAN_BUFFER_CREATION_ERROR
{
	VULKAN_BUFFER_CREATION_ERROR_UNKNOWN
};

enum VULKAN_IMAGE_CREATION_ERROR
{
	VULKAN_IMAGE_CREATION_ERROR_UNKNOWN
};

enum VmaMemoryUsage;
class ITransferHandle;
enum TRANSFER_QUEUE_GET_ERR;
struct VkDevice_T;
class ITransferOperations;
class IGVulkanDescriptorPool;
class IGVulkanUniformBuffer;
class IGVulkanGraphicPipelineState;
class IGVulkanViewport;

class ENGINE_API IGVulkanLogicalDevice
{
public:
	virtual ~IGVulkanLogicalDevice() = default;

	virtual bool init() = 0;

	virtual bool is_valid() const = 0;

	virtual void destroy() = 0;

	virtual VkDevice_T* get_vk_device() = 0;

	virtual IGVulkanPhysicalDevice* get_bounded_physical_device() = 0;

	virtual IGVulkanQueue* get_present_queue() noexcept = 0;

	virtual IGVulkanQueue* get_render_queue() noexcept = 0;

	virtual IGVulkanQueue* get_resource_queue() noexcept = 0;

	virtual bool begin_command_buffer_record(GVulkanCommandBuffer* buff) = 0;

	virtual void end_command_buffer_record(GVulkanCommandBuffer* buff) = 0;


	virtual std::expected<IVulkanBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_buffer(uint64_t size, uint32_t bufferUsageFlags, VmaMemoryUsage memoryUsageFlag) = 0;

	virtual std::expected< IGVulkanVertexBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_vertex_buffer(uint64_t size) = 0;

	virtual std::expected< IVulkanImage*, VULKAN_IMAGE_CREATION_ERROR> create_image(const VkImageCreateInfo* imageCreateInfo, VmaMemoryUsage memoryUsageFlag) = 0;

	virtual IGVulkanDevice* get_owner() noexcept = 0;


	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd() = 0;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd(uint64_t timeout) = 0;

	virtual void finish_execute_and_wait_transfer_cmd(ITransferHandle* handle) = 0;


	virtual ITransferOperations* get_transfer_operation() = 0;

	virtual IGVulkanDescriptorPool* create_and_init_default_pool(uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount) =0;

	virtual IGVulkanDescriptorPool* create_and_init_vector_pool(const std::unordered_map<VkDescriptorType, int>& typeMap) = 0;

	virtual IGVulkanPipelineLayout* create_and_init_vector_pipeline_layout(const std::vector<VkDescriptorSetLayout_T*>& layouts) = 0;

	virtual std::expected<IGVulkanUniformBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_uniform_buffer(uint32_t size) = 0;


	virtual IGVulkanGraphicPipelineState* create_vertex_input_state(const std::vector< VkVertexInputBindingDescription>* vertexBindingDescription,
		const std::vector< VkVertexInputAttributeDescription>* attributeDescription) = 0;


	virtual IGVulkanGraphicPipelineState* create_default_input_assembly_state() = 0;

	virtual IGVulkanGraphicPipelineState* create_input_assembly_state(VkPrimitiveTopology topology, bool resetAfterIndexedDraw) = 0;

	virtual IGVulkanGraphicPipelineState* create_default_rasterization_state() = 0;

	virtual IGVulkanGraphicPipelineState* create_default_none_multisample_state() = 0;

	virtual IGVulkanGraphicPipelineState* create_default_viewport_state(uint32_t width, uint32_t height) = 0;

	virtual IGVulkanGraphicPipelineState* create_default_color_blend_state() = 0;

	//X Stencil test is off for the default state
	virtual IGVulkanGraphicPipelineState* create_default_depth_stencil_state() = 0;

	virtual IGVulkanGraphicPipeline* create_and_init_default_graphic_pipeline_for_vp(IGVulkanViewport* vp,
		const std::vector< IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states) = 0;


	virtual IGVulkanGraphicPipeline* create_and_init_graphic_pipeline_injector_for_vp(IGVulkanViewport* vp, const std::vector<IVulkanShaderStage*>& shaderStages, 
		const std::vector<IGVulkanGraphicPipelineState*>& states, IGVulkanGraphicPipelineLayoutCreator* injector) = 0;
private:
};


#endif // IVULKAN_L_DEVICE_H