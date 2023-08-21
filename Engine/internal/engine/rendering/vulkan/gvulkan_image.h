#ifndef GVULKAN_IMAGE_H
#define GVULKAN_IMAGE_H

#include "engine/rendering/vulkan/ivulkan_image.h"

#include <vma/vk_mem_alloc.h>

class GVulkanLogicalDevice;

class GVulkanImage : public IVulkanImage
{
	friend class GVulkanLogicalDevice;
public:
	GVulkanImage(GVulkanLogicalDevice* owner,VmaAllocator allocator);
	~GVulkanImage();


	// Inherited via IVulkanImage
	virtual void unload() override;

	VkImage* get_pimage();
	VmaAllocation* get_pallocation();
	virtual IGVulkanLogicalDevice* get_bounded_device() override;

	virtual bool create_image_view(const VkImageViewCreateInfo* info) override;

	virtual VkImage_T* get_vk_image() override;

private:
	bool m_inited;
	VmaAllocation m_allocationBlock;
	VmaAllocator m_allocator;
	VkImageCreateInfo m_creationInfo;
	VkImage m_image;
	GVulkanLogicalDevice* m_boundedDevice;
	VkImageView m_imageView;
	VkImageViewCreateInfo m_imageViewCreationInfo;



};

#endif // GVULKAN_IMAGE_H