#ifndef IVULKAN_RENDER_PASS
#define IVULKAN_RENDER_PASS

#include "engine/GEngine_EXPORT.h"

struct VkRenderPass_T;


class ENGINE_API IGVulkanRenderPass
{
public:
	virtual ~IGVulkanRenderPass() = default;

	virtual VkRenderPass_T* get_vk_renderpass() = 0;

private:
};

#endif // IVULAKN_RENDER_PASS