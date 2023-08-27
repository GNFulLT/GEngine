#ifndef GIMGUI_DESCRIPTOR_H
#define GIMGUI_DESCRIPTOR_H

#include "engine/rendering/vulkan/ivulkan_descriptor.h"

class GImGuiDescriptorCreator;

class GImGuiDescriptorSet : public IGVulkanDescriptorSet
{
public:
	GImGuiDescriptorSet(VkDescriptorSet_T* set, GImGuiDescriptorCreator* owner);
	// Inherited via IGVulkanDescriptorSet
	virtual IGVulkanDescriptorCreator* get_creator() override;
	virtual void destroy_dtor() override;
	virtual VkDescriptorSet_T* get_vk_descriptor() override;

	VkDescriptorSet_T* get_descriptor_set();

private:
	GImGuiDescriptorCreator* m_owner;
	VkDescriptorSet_T* m_descriptorSet;
};

#endif // GIMGUI_DESCRIPTOR_H