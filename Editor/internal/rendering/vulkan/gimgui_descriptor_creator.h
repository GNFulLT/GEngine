#ifndef GIMGUI_DESCRIPTOR_CREATOR_H
#define GIMGUI_DESCRIPTOR_CREATOR_H

#include "engine/rendering/vulkan/ivulkan_descriptor_creator.h"

class GImGuiDescriptorCreator : public IGVulkanDescriptorCreator
{
public:
	GImGuiDescriptorCreator(IGVulkanLogicalDevice* dev);

	// Inherited via IGVulkanDescriptorCreator
	virtual std::expected<IGVulkanDescriptorSet*, std::string> create_descriptor_set_for_texture(IVulkanImage* image, VkSampler_T* sampler) override;

	virtual void destroy_descriptor_set_dtor(IGVulkanDescriptorSet* set) override;

	virtual IGVulkanLogicalDevice* get_bounded_device() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;


};

#endif // GIMGUI_DESCRIPTOR_CREATOR_H