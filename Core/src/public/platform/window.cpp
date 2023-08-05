#include "public/platform/window.h"

#include "internal/platform/window_glfw.h"

CORE_API Window* create_default_window()
{
	return WindowGLFW::create_default_window();
}