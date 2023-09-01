#ifndef VULKAN_VIEWPORT_H
#define VULKAN_VIEWPORT_H

#include <cstdint>
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "internal/engine/rendering/vulkan/vulkan_renderpass.h"

#include <vector>

class GVulkanLogicalDevice;
class IGVulkanQueue;
struct VkSurfaceKHR_T;
struct VkSurfaceFormatKHR;
struct VkImageView_T;
class GVulkanMainViewport : public IGVulkanViewport
{
public:
	GVulkanMainViewport(GVulkanLogicalDevice* inDevice,uint32_t sizeX, uint32_t sizeY);
	~GVulkanMainViewport();

	bool init(int width,int height,int format);

	void set_image_views_ref(const std::vector<VkImageView_T*>* imageViews);

	void set_current_image(uint32_t index);

	void destroy();


	virtual void* get_vk_current_image_renderpass() override;

	virtual uint32_t get_width() const override;

	virtual uint32_t get_height() const override;


	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) override;
	

	virtual bool can_be_used_as_texture() override;

	// If the viewport couldn't be used as texture it returns null
	virtual IGVulkanDescriptorSet* get_descriptor() override;

	// Inherited via IGVulkanViewport
	virtual IGVulkanRenderPass* get_render_pass() override;
	GVulkanLogicalDevice* m_device;
private:
	GVulkanRenderpass m_renderpass;
	uint32_t m_sizeX;
	uint32_t m_sizeY;
	uint32_t m_currentImage;
	const std::vector<VkImageView_T*>* m_imageViews;

};

#endif //VULKAN_VIEWPORT_H