#ifndef GIMGUI_SCENE_WINDOW_H
#define GIMGUI_SCENE_WINDOW_H


#include "editor/igimgui_window_impl.h"
#include <string>
#include <glm/glm.hpp>

class IGSceneManager;

class GImGuiSceneWindow : public IGImGuiWindowImpl
{
public:
	GImGuiSceneWindow();

	// Inherited via IGImGuiWindowImpl
	virtual bool init() override;

	virtual void set_storage(GImGuiWindowStorage* storage) override;

	virtual bool need_render() override;

	virtual void render() override;

	virtual void on_resize() override;

	virtual void on_data_update() override;

	virtual void destroy() override;

	virtual const char* get_window_name() override;

	uint32_t get_selected_entity() const noexcept;
private:

private:
	std::string m_name;
	uint32_t m_selectedEntity;
	IGSceneManager* m_sceneManager;


};

#endif // GIMGUI_SCENE_WINDOW_H