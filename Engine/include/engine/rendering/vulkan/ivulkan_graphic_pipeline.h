#ifndef IVULKAN_GRAPHIC_PIPELINE_H
#define IVULKAN_GRAPHIC_PIPELINE_H

#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include <vector>

struct VkPipeline_T;
class IGVulkanPipelineLayout;
class GVulkanCommandBuffer;
class IGVulkanGraphicPipeline
{
public:
	virtual ~IGVulkanGraphicPipeline() = default;

	virtual const std::vector<IVulkanShaderStage*>* get_shader_stages() = 0;
	
	virtual VkPipeline_T* get_pipeline() = 0;
	
	virtual IGVulkanPipelineLayout* get_pipeline_layout() = 0;

	virtual void destroy() = 0;

	//X TODO : WILL BE ERASED HERE
	virtual void bind_sets(GVulkanCommandBuffer* cmd,uint32_t frameIndex) {};
private:
};

#endif // IVULKAN_GRAPHIC_PIPELINE_h