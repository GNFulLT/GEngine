#ifndef IVULKAN_SHADER_STAGE_H
#define IVULKAN_SHADER_STAGE_H

struct VkPipelineShaderStageCreateInfo;

#include "engine/GEngine_EXPORT.h"

class IGShaderResource;
struct VkGraphicsPipelineCreateInfo;

class ENGINE_API IVulkanShaderStage
{
public:
	virtual ~IVulkanShaderStage() = default;
	
	virtual const VkPipelineShaderStageCreateInfo* get_creation_info() = 0;

	virtual IGShaderResource* get_shader_resource() = 0;
	
	virtual bool is_valid() = 0;
private:
};

#endif // IVULKAN_SHADER_STAGE_H