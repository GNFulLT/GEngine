#ifndef GVULKAN_DEFAULT_SAMPLER_CREATOR_H
#define GVULKAN_DEFAULT_SAMPLER_CREATOR_H

#include "engine/rendering/vulkan/ivulkan_sampler_creator.h"
#include "engine/manager/igpipeline_object_manager.h"
class GDefaultSamplerCreator : public IGVulkanSamplerCreator
{
public:
	GDefaultSamplerCreator(IGPipelineObjectManager* obj);
	virtual std::expected<IGVulkanSampler*, std::string> create_sampler(IGVulkanLogicalDevice* dev) override;
	// Inherited via IGVulkanSamplerCreator
	virtual void destroy_sampler(IGVulkanSampler* sampler) override;
	virtual const char* get_name() override;
private:
	IGPipelineObjectManager* m_objManager;
	GSharedPtr<IGVulkanNamedSampler> m_maxPerformantSampler;

};

#endif // GVULKAN_DEFAULT_SAMPLER_CREATOR_H