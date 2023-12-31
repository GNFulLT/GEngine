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
	virtual IGVulkanNamedPipelineLayout* get_named_pipeline_layout(const char* name) override;

	virtual IGVulkanNamedSetLayout* get_named_set_layout(const char* name) override;

	virtual IGVulkanNamedSetLayout* create_or_get_named_set_layout(const char* name, VkDescriptorSetLayoutCreateInfo* createInfo);

private:
	bool init_named_objects();
	void destroy_named_objects();

	bool init_named_renderpass();
	void destroy_named_renderpass();

	bool init_named_sampler();
	void destroy_named_sampler();

	bool init_named_pipeline_layouts();
	void destroy_named_pipeline_layouts();

	bool init_named_set_layouts();
	void destroy_named_set_layouts();

	bool init_named_compute_pipelines();
	void destroy_named_compute_pipelines();
	

	VkFormat m_swapchainFormat;
	uint32_t m_framesInFlight;
	IGVulkanLogicalDevice* m_logicalDevice;
	ankerl::unordered_dense::segmented_map<std::string, GSharedPtr<IGVulkanNamedRenderPass>> m_namedRenderpassMap;
	ankerl::unordered_dense::segmented_map<std::string, GSharedPtr<IGVulkanNamedSampler>> m_namedSamplerMap;
	ankerl::unordered_dense::segmented_map<std::string, GSharedPtr<IGVulkanNamedPipelineLayout>> m_namedPipelineLayoutMap;
	ankerl::unordered_dense::segmented_map<std::string, GSharedPtr<IGVulkanNamedSetLayout>> m_namedSetLayoutMap;


	

	// Inherited via IGPipelineObjectManager
	virtual IGVulkanNamedRenderPass* create_or_get_named_renderpass(const char* name, VkRenderPassCreateInfo* createInfo) override;


	// Inherited via IGPipelineObjectManager
	virtual IGVulkanNamedPipelineLayout* create_or_get_named_pipeline_layout(const char* name, VkPipelineLayoutCreateInfo* createInfo) override;


	// Inherited via IGPipelineObjectManager
	virtual IGVulkanNamedGraphicPipeline* create_named_graphic_pipeline(const char* name, IGVulkanNamedRenderPass* renderPass) override;

};

#endif // GPIPELINE_OBJECT_MANAGER_H