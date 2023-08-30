#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

class IGVulkanDevice;
class IGVulkanApp;
class Window;
class IGVulkanViewport;
struct VkDescriptorPool_T;
class GVulkanCommandBuffer;
class ImGuiWindowManager;
class IGVulkanViewport;
class GImGuiViewportWindow;
class GSceneRenderer;
class GImGuiTextEditorWindow;
class GImGuiContentBrowserWindow;
class GImGuiLogWindow;

class ImGuiLayer
{
public:
	ImGuiLayer(IGVulkanViewport* viewport,Window* window,IGVulkanApp* app,IGVulkanDevice* dev);
	~ImGuiLayer();

	bool init();

	void destroy();

	bool before_render();
	void render(GVulkanCommandBuffer* cmd);

	void set_viewport(IGVulkanViewport* viewport);

	ImGuiWindowManager* get_window_manager();

	GImGuiLogWindow* get_log_window();
private:
	IGVulkanDevice* m_dev;
	Window* m_window;
	IGVulkanApp* m_app;
	VkDescriptorPool_T* m_descriptorPool;
	IGVulkanViewport* m_viewport;
	ImGuiWindowManager* m_windowManager;
	GImGuiLogWindow* m_logWindow;
	// Builtin windows
	GImGuiViewportWindow* m_renderViewportWindow;
	GImGuiContentBrowserWindow* m_contentBrowserWindow;
	GSceneRenderer* m_sceneRenderer;
};

#endif // IMGUI_LAYER_H