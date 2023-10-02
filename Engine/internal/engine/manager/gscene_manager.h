#ifndef GSCENE_MANAGER_H
#define GSCENE_MANAGER_H

#include "engine/manager/igscene_manager.h"
#include <vector>
#include "internal/engine/rendering/renderer/gscene_renderer2.h"

struct VkDescriptorSet_T;

class IGCameraManager;
class IGVulkanLogicalDevice;
class IGVulkanUniformBuffer;

class GSceneManager : public IGSceneManager
{
public:
	struct GlobalUniformBuffer
	{
		float viewProj[16];
		float pos[3];
		float view[16];
		float resolution[2];
	};
	// Inherited via IGSceneManager
	virtual IGVulkanUniformBuffer* get_global_buffer_for_frame(uint32_t frame) const noexcept override;
	virtual void reconstruct_global_buffer_for_frame(uint32_t frame) override;
	virtual bool init(uint32_t framesInFlight) override;
	virtual void destroy() override;
	virtual bool init_deferred_renderer(IGVulkanNamedDeferredViewport* deferred) override;
	virtual bool is_renderer_active() override;

private:
	GlobalUniformBuffer m_globalData;
	IGCameraManager* m_cameraManager;
	IGVulkanLogicalDevice* m_logicalDevice;
	uint32_t m_framesInFlight;
	std::vector<IGVulkanUniformBuffer*> m_globalBuffers;
	std::vector<void*> m_globalBufferMappedMem;
	IGVulkanNamedDeferredViewport* m_deferredTargetedViewport = nullptr;
	
	IGVulkanNamedSetLayout* m_globalDataSetLayout;
	IGVulkanDescriptorPool* m_globalPool;

	GSceneRenderer2* m_deferredRenderer = nullptr;

	std::vector<VkDescriptorSet_T*> m_globalSets;

	// Inherited via IGSceneManager
	virtual IGVulkanDeferredRenderer* get_deferred_renderer() const noexcept override;
	virtual VkDescriptorSet_T* get_global_set_for_frame(uint32_t frame) const noexcept override;
	// Inherited via IGSceneManager
	virtual IGVulkanNamedDeferredViewport* create_default_deferred_viewport(IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass, VkFormat compositionFormat) override;
};

#endif // GSCENE_MANAGER_H