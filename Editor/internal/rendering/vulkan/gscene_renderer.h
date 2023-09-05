#ifndef GSCENE_RENDERER_H
#define GSCENE_RENDERER_H
#include "volk.h"

class IGVulkanViewport;
class IGVulkanDevice;
class GVulkanCommandBuffer;
class GVulkanSemaphore;
class IGShaderResource;
class IVulkanShaderStage;
class IGVulkanGraphicPipeline;
#include "internal/rendering/renderable.h"

#include "public/core/templates/shared_ptr.h"
class GSceneRenderer
{
public:
	GSceneRenderer(IGVulkanViewport* viewport, IGVulkanDevice* device);

	void render_the_scene();
	
	bool init();
	void destroy();


	void set_the_viewport(IGVulkanViewport* viewport);
private:
	IGVulkanDevice* m_device;
	IGVulkanViewport* m_viewport;
	std::vector< std::vector<GVulkanCommandBuffer*>> m_frameCmds;
	std::vector< GVulkanSemaphore*> m_frameSemaphores;
	std::vector<uint32_t> m_currentCmdIndex;

	VkViewport m_vkViewport;
	VkRect2D m_vkScissor;

	GSharedPtr< IGShaderResource> m_basicFragShader;
	GSharedPtr< IGShaderResource> m_basicVertexShader;
	IGVulkanGraphicPipeline* m_graphicPipeline = nullptr;
	IVulkanShaderStage* m_fragShaderStage = nullptr;
	IVulkanShaderStage* m_vertexShaderStage = nullptr;

	Renderable* triangle;
};

#endif // GSCENE_RENDERER_H