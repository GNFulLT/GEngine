#include "volk.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_renderpass.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"

GNamedVulkanRenderPass::GNamedVulkanRenderPass(IGVulkanLogicalDevice* device,const char* name, VkRenderPass_T* renderPass,int format)
{
	m_renderPassName = name;
	m_renderPass = renderPass;
	m_boundedDevice = device;
	m_format = format;
}

VkRenderPass_T* GNamedVulkanRenderPass::get_vk_renderpass()
{
	return m_renderPass;
}

void GNamedVulkanRenderPass::destroy()
{
	vkDestroyRenderPass(m_boundedDevice->get_vk_device(),m_renderPass,nullptr);
}

int GNamedVulkanRenderPass::get_supported_render_format()
{
	return m_format;
}
