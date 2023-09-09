#ifndef GVULKAN_NAMED_RENDERPASS_H
#define GVULKAN_NAMED_RENDERPASS_H

#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
#include <string>
class IGVulkanLogicalDevice;
class GNamedVulkanRenderPass : public IGVulkanNamedRenderPass
{
public:
	GNamedVulkanRenderPass(IGVulkanLogicalDevice* device,const char* name,VkRenderPass_T* renderPass,int format);
	virtual VkRenderPass_T* get_vk_renderpass() override;
	
	virtual int get_supported_render_format() override;

	void destroy();
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	std::string m_renderPassName;
	VkRenderPass_T* m_renderPass;
	int m_format;

};

#endif // GVULKAN_NAMED_RENDERPASS_H