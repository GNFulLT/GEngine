#include "volk.h"
#include "internal/engine/rendering/vulkan/gvulkan_shader_stage.h"
#include "engine/resource/igshader_resource.h"
#include "internal/engine/shader/spirv_shader_utils.h"

GVulkanShaderStage::GVulkanShaderStage(GSharedPtr<IGShaderResource> shaderRes)
{
	m_shaderResource = shaderRes;
	m_createInfo = {};
	m_createInfo.pNext = nullptr;
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_createInfo.module = m_shaderResource->get_vk_shader_module();
	m_createInfo.pName = m_shaderResource->get_entry_point_name();
	m_createInfo.stage = spirv_stage_to_vk_stage(m_shaderResource->get_shader_stage());
}

const VkPipelineShaderStageCreateInfo* GVulkanShaderStage::get_creation_info()
{
	return &m_createInfo;
}

IGShaderResource* GVulkanShaderStage::get_shader_resource()
{
	return m_shaderResource.get();
}

bool GVulkanShaderStage::is_valid()
{
	return m_shaderResource.is_valid() && m_shaderResource->get_resource_state() == RESOURCE_LOADING_STATE_LOADED;
}
