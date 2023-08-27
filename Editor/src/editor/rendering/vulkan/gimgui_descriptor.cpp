#include "internal/rendering/vulkan/gimgui_descriptor.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"

GImGuiDescriptorSet::GImGuiDescriptorSet(VkDescriptorSet_T* set, GImGuiDescriptorCreator* owner)
{
	m_owner = owner;
	m_descriptorSet = set;
}

IGVulkanDescriptorCreator* GImGuiDescriptorSet::get_creator()
{
	return m_owner;
}

void GImGuiDescriptorSet::destroy_dtor()
{
	m_owner->destroy_descriptor_set_dtor(this);
}

VkDescriptorSet_T* GImGuiDescriptorSet::get_vk_descriptor()
{
	return m_descriptorSet;
}

VkDescriptorSet_T* GImGuiDescriptorSet::get_descriptor_set()
{
	return m_descriptorSet;
}
