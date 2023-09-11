#include "internal/rendering/vulkan/gvulkan_pipeline_layout_ref.h"

GVulkanPipelineLayoutRef::GVulkanPipelineLayoutRef(VkPipelineLayout_T* layout)
{
	m_layout = layout;
}

VkPipelineLayout_T* GVulkanPipelineLayoutRef::get_vk_pipeline_layout()
{
	return m_layout;
}

void GVulkanPipelineLayoutRef::destroy()
{
}
