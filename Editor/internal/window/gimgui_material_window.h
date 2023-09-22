#ifndef GIMGUI_MATERIALS_WINDOW_H
#define GIMGUI_MATERIALS_WINDOW_H


#include "editor/igimgui_window_impl.h"
#include <string>
#include "engine/rendering/material/gmaterial.h"

class GImGuiMaterialsWindow : public IGImGuiWindowImpl
{
public:
	GImGuiMaterialsWindow();

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

	
};

#endif // GIMGUI_MATERIALS_WINDOW_H