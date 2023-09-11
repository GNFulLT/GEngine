#ifndef GGRID_RENDERER_H
#define GGRID_RENDERER_H

class IGResourceManager;
class IGTextureResource;
class IGVulkanDescriptorPool;
class IGCameraManager;
class IGVulkanViewport;
class IGVulkanShaderStage;
class IGShaderResource;
class IVulkanShaderStage;
class IGShaderManager;
class IGVulkanGraphicPipeline;
class GVulkanCommandBuffer;
class IGVulkanViewport;
class IGVulkanLogicalDevice;
class GGridPipelineLayoutCreator;

#include "public/math/gtransform.h"
#include "engine/manager/igpipeline_object_manager.h"
#include <cstdint>
#include "internal/rendering/grid_spec.h"

class GImGuiGridSettingsWindow;

class GridRenderer
{
	friend class GImGuiGridSettingsWindow;
public:
	GridRenderer(IGVulkanLogicalDevice* boundedDevice, IGResourceManager* mng, IGCameraManager* cameraManager,
		IGPipelineObjectManager* obj, IGVulkanViewport* viewport, IGShaderManager* shaderMng, uint32_t framesInFlight);

	bool init();

	void destroy();
	
	bool wants_render() const noexcept;
	
	void render(GVulkanCommandBuffer* cmd,uint32_t frameIndex);
private:
	GridSpec* get_spec() noexcept;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGCameraManager* p_cameraManager;
	IGVulkanViewport* p_renderingViewport;
	IGShaderManager* p_shaderManager;
	IGPipelineObjectManager* p_pipelineManager;
	uint32_t m_framesInFlight;


	IGShaderResource* m_gridFrag;
	IGShaderResource* m_gridVert;

	IVulkanShaderStage* m_gridVertStage;
	IVulkanShaderStage* m_gridFragStage;

	IGVulkanGraphicPipeline* m_pipeline;
	GGridPipelineLayoutCreator* m_pipelineLayoutCreator;

	gtransform transform;

	bool m_wantsRender = true;

	GridSpec m_spec;
};

#endif // GGRID_RENDERER_H