#ifndef GSCENE_RENDERER_H
#define GSCENE_RENDERER_H

class IGVulkanViewport;
class IGVulkanDevice;
class GVulkanCommandBuffer;
class GVulkanSemaphore;
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
	GVulkanCommandBuffer* m_cmd;
	GVulkanSemaphore* m_semaphore;
};

#endif // GSCENE_RENDERER_H