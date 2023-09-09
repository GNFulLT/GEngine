#ifndef GPIPELINE_OBJECT_MANAGER_H
#define GPIPELINE_OBJECT_MANAGER_H

#include "public/core/templates/unordered_dense.h"
#include "public/core/templates/shared_ptr.h"

#include <string>
#include <cstdint>
#include "engine/manager/igpipeline_object_manager.h"

enum VKFormat;
class IGVulkanLogicalDevice;

class GPipelineObjectManager : public IGPipelineObjectManager
{
public:
	GPipelineObjectManager(IGVulkanLogicalDevice* logicalDevice,VkFormat swapchainFormat,uint32_t framesInFlight);
	virtual bool init() override;
	virtual void destroy() override;

	virtual GSharedPtr<IGVulkanNamedRenderPass> get_named_renderpass(const char* name) override;
	virtual GSharedPtr<IGVulkanNamedSampler> get_named_sampler(const char* name) override;

private:
	bool init_named_objects();
	void destroy_named_objects();

	bool init_named_renderpass();
	void destroy_named_renderpass();

	bool init_named_sampler();
	void destroy_named_sampler();

	VkFormat m_swapchainFormat;
	uint32_t m_framesInFlight;
	IGVulkanLogicalDevice* m_logicalDevice;
	ankerl::unordered_dense::segmented_map<std::string, GSharedPtr<IGVulkanNamedRenderPass>> m_namedRenderpassMap;
	ankerl::unordered_dense::segmented_map<std::string, GSharedPtr<IGVulkanNamedSampler>> m_namedSamplerMap;

};

#endif // GPIPELINE_OBJECT_MANAGER_H