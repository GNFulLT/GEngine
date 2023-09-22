#ifndef GIMGUI_VIEWPORT_WINDOW_H
#define GIMGUI_VIEWPORT_WINDOW_H

#include "editor/igimgui_window_impl.h"

#include <string>
#include <glm/glm.hpp>
class IGVulkanViewport;
class GImGuiWindowStorage;
class GImGuiSceneWindow;
class IGCameraManager;

class GImGuiViewportWindow : public IGImGuiWindowImpl
{
public:
	GImGuiViewportWindow();

	void set_the_viewport(IGVulkanViewport* viewport);
	
	// Inherited via IGImGuiWindowImpl
	virtual bool init() override;
	virtual void set_storage(GImGuiWindowStorage* storage) override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual const char* get_window_name() override;
	virtual void destroy() override;
private:
	std::string m_name;
	GImGuiWindowStorage* m_storage;
	IGVulkanViewport* m_viewport;
	GImGuiSceneWindow* m_sceneWindow;
	IGCameraManager* m_cameraManager;
	uint32_t m_selectedNode = -1;
	glm::mat4 m_selectedTranslation;
	bool m_initedTheViewportFirstTime;
};

#endif // GIMGUI_VIEWPORT_WINDOW_H