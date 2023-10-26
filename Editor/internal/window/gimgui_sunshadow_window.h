#ifndef GIMGUI_SUN_SHADOW_PORT_H
#define GIMGUI_SUN_SHADOW_PORT_H


#include "editor/igimgui_window_impl.h"
#include <string>
#include <glm/glm.hpp>

class IGVulkanDescriptorSet;
class Scene;

class GImGuiSunShadowPortWindow : public IGImGuiWindowImpl
{
public:
	GImGuiSunShadowPortWindow();
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
	std::string m_name;
	GImGuiWindowStorage* m_storage;
	bool m_resizedAtThisFrame = false;
};
#endif 