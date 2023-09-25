#ifndef IGPIPELINE_OBJECT_MANAGER_H
#define IGPIPELINE_OBJECT_MANAGER_H

#include <string_view>
#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
#include "engine/rendering/vulkan/named/igvulkan_named_sampler.h"
#include "engine/rendering/vulkan/named/igvulkan_named_pipeline_layout.h"
#include "engine/rendering/vulkan/named/igvulkan_named_set_layout.h"

struct VkDescriptorSetLayoutCreateInfo;

class IGPipelineObjectManager
{
public:
	inline static constexpr std::string_view RENDER_DEPTH_PASS = "render_depth_pass";
	inline static constexpr std::string_view MAX_PERFORMANT_SAMPLER = "max_performant_sampler";
	inline static constexpr std::string_view CAMERA_PIPE_LAYOUT = "camera_pipe_layout";
	inline static constexpr std::string_view UVERT_LAYOUT = "uvert_set_layout";

	virtual ~IGPipelineObjectManager() = default;

	virtual bool init() = 0;

	virtual void destroy() = 0;


	virtual GSharedPtr<IGVulkanNamedRenderPass> get_named_renderpass(const char* name) = 0;
	virtual GSharedPtr<IGVulkanNamedSampler> get_named_sampler(const char* name) = 0;
	virtual IGVulkanNamedPipelineLayout* get_named_pipeline_layout(const char* name) = 0;
	virtual IGVulkanNamedSetLayout* get_named_set_layout(const char* name) = 0;
	//X If there is already created layout with this createInfo it will return it. Name will not be changed in that case so the name given in the argument will be added to indirect pointers
	virtual IGVulkanNamedSetLayout* create_or_get_named_set_layout(const char* name, VkDescriptorSetLayoutCreateInfo* createInfo)= 0;

private:
};

#endif // IGPIPELINE_OBJECT_MANAGER_H