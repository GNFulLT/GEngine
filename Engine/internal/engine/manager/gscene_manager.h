#ifndef GSCENE_MANAGER_H
#define GSCENE_MANAGER_H

#include "engine/manager/igscene_manager.h"
#include <vector>

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
	};
	// Inherited via IGSceneManager
	virtual IGVulkanUniformBuffer* get_global_buffer_for_frame(uint32_t frame) const noexcept override;
	virtual void reconstruct_global_buffer_for_frame(uint32_t frame) override;
	virtual bool init(uint32_t framesInFlight) override;

private:
	GlobalUniformBuffer m_globalData;

	IGCameraManager* m_cameraManager;
	IGVulkanLogicalDevice* m_logicalDevice;
	uint32_t m_framesInFlight;
	std::vector<IGVulkanUniformBuffer*> m_globalBuffers;
	std::vector<void*> m_globalBufferMappedMem;

	// Inherited via IGSceneManager
	virtual void destroy() override;
};

#endif // GSCENE_MANAGER_H