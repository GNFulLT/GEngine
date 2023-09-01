#ifndef IVULKAN_GRAPHIC_PIPELINE_H
#define IVULKAN_GRAPHIC_PIPELINE_H

#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include <vector>

class IGVulkanGraphicPipeline
{
public:
	virtual ~IGVulkanGraphicPipeline() = default;

	virtual const std::vector<IVulkanShaderStage*>* get_shader_stages() = 0;
private:
};

#endif // IVULKAN_GRAPHIC_PIPELINE_h