#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "internal/rendering/vulkan/gimgui_descriptor.h"

GImGuiDescriptorCreator::GImGuiDescriptorCreator(IGVulkanLogicalDevice* dev)
{
	m_boundedDevice = dev;
}

std::expected<IGVulkanDescriptorSet*, std::string> GImGuiDescriptorCreator::create_descriptor_set_for_texture(IVulkanImage* image, VkSampler_T* sampler)
{
	VkDescriptorSet set = ImGui_ImplVulkan_AddTexture(sampler, image->get_vk_image_view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	//X TODO GDNEWDA
	return new GImGuiDescriptorSet(set, this);
}

void GImGuiDescriptorCreator::destroy_descriptor_set_dtor(IGVulkanDescriptorSet* set)
{
	//X TODO SAFE CAST
	// UNSAFE CAST
	GImGuiDescriptorSet* unsafe = (GImGuiDescriptorSet*)set;

	VkDescriptorSet dset = unsafe->get_descriptor_set();
	ImGui_ImplVulkan_RemoveTexture(dset);
	delete set;
}

IGVulkanLogicalDevice* GImGuiDescriptorCreator::get_bounded_device()
{
	return m_boundedDevice;
}
