#ifndef GIMGUI_COMPOSITION_PORT_H
#define GIMGUI_COMPOSITIONL_PORT_H


#include "editor/igimgui_window_impl.h"
#include <string>
#include <glm/glm.hpp>

class GImGuiSceneWindow;
class IGVulkanDescriptorSet;
class Scene;
class IGCameraManager;
class IGSceneManager;

class GImGuiCompositionPortWindow : public IGImGuiWindowImpl
{
public:
	enum TRANSFORM_TYPE
	{
		TRANSFORM_TYPE_TRANSLATE,
		TRANSFORM_TYPE_SCALE,
		TRANSFORM_TYPE_ROTATE
	};


	GImGuiCompositionPortWindow();
	// Inherited via IGImGuiWindowImpl
	virtual bool init() override;
	virtual void set_storage(GImGuiWindowStorage* storage) override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual void destroy() override;
	virtual const char* get_window_name() override;


private:
	uint32_t transformTypeToImgui(TRANSFORM_TYPE type);
private:
	TRANSFORM_TYPE tType = TRANSFORM_TYPE_TRANSLATE;
	std::string m_name;
	bool m_resizedAtThisFrame = false;
	GImGuiWindowStorage* m_storage;
	GImGuiSceneWindow* m_sceneWindow;
	IGCameraManager* m_cameraManager;
	IGSceneManager* m_sceneManager;
	int m_selectedNode = -1;
};
#endif 