#ifndef GIMGUI_PROPERTIES_WINDOW_H
#define GIMGUI_PROPERTIES_WINDOW_H


#include "editor/igimgui_window_impl.h"
#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
	
class IGCameraManager;
class GImGuiSceneWindow;

class GImGuiPropertiesWindow : public IGImGuiWindowImpl
{
public:
	GImGuiPropertiesWindow();

	virtual bool init() override;
	virtual void set_storage(GImGuiWindowStorage* storage) override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual void destroy() override;
	virtual const char* get_window_name() override;
private:
	std::string m_name;
	glm::mat4 m_frustrumMatrix;
	
	IGCameraManager* m_cam;
	GImGuiSceneWindow* m_sceneWindow;
	float m_nearPlane = 0.001f;
	float m_farPlane = 150.f;
	float m_lodBase;
	float m_lodStep;
	uint32_t m_currentSelectedNode;

	glm::vec3 m_currentNodePosition;
	glm::quat m_currentNodeRotatition;
	glm::vec3 m_currentNodeScale;
};

#endif // GIMGUI_PROPERTIES_WINDOW_H