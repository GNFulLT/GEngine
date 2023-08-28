#include "public/platform/window.h"

#include "internal/platform/window_glfw.h"

CORE_API Window* create_default_window()
{
	return WindowGLFW::create_default_window();
}

CORE_API GSharedPtr<WindowProps> get_default_props()
{
	WindowProps* props = gdnew(WindowProps);
	GSharedPtr<WindowProps> pProps(props);
	pProps->height = 480;
	pProps->width = 640;
	pProps->title = std::string("GEngine");
	return pProps;
}

CORE_API Window* create_with_props(GSharedPtr<WindowProps> props)
{
	return gdnewa(WindowGLFW,props);
}
