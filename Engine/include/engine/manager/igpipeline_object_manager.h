#ifndef IGPIPELINE_OBJECT_MANAGER_H
#define IGPIPELINE_OBJECT_MANAGER_H

#include <string_view>
#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"


class IGPipelineObjectManager
{
public:
	inline static constexpr std::string_view RENDER_DEPTH_PASS = "render_depth_pass";

	virtual ~IGPipelineObjectManager() = default;

	virtual bool init() = 0;

	virtual void destroy() = 0;


	virtual GSharedPtr<IGVulkanNamedRenderPass> get_named_renderpass(const char* name) = 0;
private:
};

#endif // IGPIPELINE_OBJECT_MANAGER_H