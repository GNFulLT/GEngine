#ifndef GVULKAN_SHADER_STAGE_H
#define GVULKAN_SHADER_STAGE_H

#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "volk.h"

class IGShaderResource;

class GVulkanShaderStage : public IVulkanShaderStage
{
public:
	GVulkanShaderStage(GSharedPtr<IGShaderResource> shaderRes);


	// Inherited via IVulkanShaderStage
	virtual const VkPipelineShaderStageCreateInfo* get_creation_info() override;
	virtual IGShaderResource* get_shader_resource() override;

	// Inherited via IVulkanShaderStage
	virtual bool is_valid() override;
private:
	GSharedPtr<IGShaderResource> m_shaderResource;
	VkPipelineShaderStageCreateInfo m_createInfo;


};

#endif // GVULKAN_SHADER_STAGE