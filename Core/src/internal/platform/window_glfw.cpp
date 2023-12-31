#include "internal/platform/window_glfw.h"
#include "public/core/templates/memnewd.h"

WINDOW_MODE WindowGLFW::get_window_mode() const noexcept
{
	return m_props->mode;
}

bool WindowGLFW::is_visible() const noexcept
{
	return m_props->isVisible;
}

void* WindowGLFW::get_native_handler() const noexcept
{
	return m_window;
}

uint32_t WindowGLFW::init()
{
	if (!glfwInit())
		return -1;


	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	// Vulkan
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if(!m_props->hasCaption)
		glfwWindowHint(GLFW_DECORATED,GLFW_FALSE);
	
	switch (m_props->mode)
	{
	case WINDOW_MODE_WINDOWED:
		m_window = glfwCreateWindow(m_props->width, m_props->height, m_props->title.c_str(), m_monitor, NULL);
		break;
		// Not Supported Yet
	default:
		m_window = glfwCreateWindow(m_props->width, m_props->height, m_props->title.c_str(), m_monitor, NULL);
		break;
	}

	m_mouseManager = std::unique_ptr<GLFWMouseManager>(new GLFWMouseManager(m_window));
	m_keyboardManager = std::unique_ptr<GLFWKeyboardManager>(new GLFWKeyboardManager());

	glfwSetWindowUserPointer(m_window, this);

	auto resizeCallback = [](GLFWwindow* window, int width, int height) {
		((WindowGLFW*)glfwGetWindowUserPointer(window))->on_resize(width, height);

	};

	auto iconifyCallback = [](GLFWwindow* window, int iconified)
	{
		((WindowGLFW*)glfwGetWindowUserPointer(window))->on_iconify_changed(iconified);

	};

	auto moveCallback = [](GLFWwindow* window, int xpos, int ypos)
	{
		((WindowGLFW*)glfwGetWindowUserPointer(window))->on_move(xpos, ypos);
	};

	auto maximizeCallback = [](GLFWwindow* window, int maximized)
	{
		((WindowGLFW*)glfwGetWindowUserPointer(window))->on_maximized(maximized);
	};

	auto mouseMove = [](GLFWwindow* window, double x,double y)
	{
		((WindowGLFW*)glfwGetWindowUserPointer(window))->on_mouse_move(x,y);
	};

	auto mouseClick = [](GLFWwindow* window, int button, int action, int mods)
	{
		((WindowGLFW*)glfwGetWindowUserPointer(window))->on_mouse_click(button,action,mods);
	};

	auto keyCallback = [](GLFWwindow* window,int key,int scancode,int action,int code)
	{
		((WindowGLFW*)glfwGetWindowUserPointer(window))->on_key(key,scancode,action,code);
	};
	
	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetWindowIconifyCallback(m_window, iconifyCallback);
	glfwSetWindowSizeCallback(m_window, resizeCallback);
	glfwSetWindowPosCallback(m_window,moveCallback);
	glfwSetWindowMaximizeCallback(m_window, maximizeCallback);
	glfwSetCursorPosCallback(m_window, mouseMove);
	glfwSetMouseButtonCallback(m_window, mouseClick);
	return 0;
}

void WindowGLFW::pump_messages()
{
	glfwPollEvents();
}

const WindowProps& WindowGLFW::get_window_props() const noexcept
{
	return *m_props.operator->();
}

bool WindowGLFW::wants_close() const
{
	return glfwWindowShouldClose(m_window);
}

GLFWMouseManager* WindowGLFW::get_glfw_mouse()
{
	return m_mouseManager.get();
}

IMouseManager* WindowGLFW::get_mouse_manager() const
{
	return m_mouseManager.get();
}

Window* WindowGLFW::create_default_window()
{
	WindowProps* props = gdnew(WindowProps);
	GSharedPtr<WindowProps> pProps(props);
	pProps->height = 480;
	pProps->width = 640;
	pProps->title = std::string("GEngine");
	return gdnewa(WindowGLFW,pProps);
}

void WindowGLFW::on_resize(int width, int height)
{
	m_props->width = width;
	m_props->height = height;
}

void WindowGLFW::on_iconify_changed(int iconified)
{
	static WINDOW_MODE lastMode;
	if (iconified)
	{
		lastMode = m_props->mode;
		m_props->mode = WINDOW_MODE_MINIMIZED;
	}
	else
	{
		m_props->mode = lastMode;


	}
}

void WindowGLFW::on_move(int x, int y)
{
	m_props->posx = x;
	m_props->posy = y;
}

void WindowGLFW::on_maximized(int maximized)
{
	if (maximized == GLFW_TRUE)
	{
		m_props->mode = WINDOW_MODE_WINDOWED_FULLSCREEN;
	}
	else
	{
		m_props->mode = WINDOW_MODE_WINDOWED;
	}
}

void WindowGLFW::on_mouse_move(int x, int y)
{
	m_mouseManager->on_mouse_move(x, y);
}

void WindowGLFW::on_mouse_click(int button, int action, int mods)
{
	m_mouseManager->on_mouse_click(button, action, mods);
}

void WindowGLFW::on_key(int key, int scancode, int action, int code)
{	
	if (action == GLFW_PRESS)
	{
		m_keyboardManager->key_pressed((KEY_CODE)key);
	}
	else if(action == GLFW_RELEASE)
	{
		m_keyboardManager->key_released((KEY_CODE)key);
	}
}

IKeyboardManager* WindowGLFW::get_keyboard_manager() const
{
	return m_keyboardManager.get();
}

void WindowGLFW::resize(uint32_t x, uint32_t y)
{
	glfwSetWindowSize(m_window,x, y);
}

void WindowGLFW::move_to(uint32_t x, uint32_t y)
{
	glfwSetWindowPos(m_window, x, y);
}

void WindowGLFW::move_by(uint32_t x, uint32_t y)
{
	int w_posx, w_posy;
	glfwGetWindowPos(m_window, &w_posx, &w_posy);
	glfwSetWindowPos(m_window, w_posx + x, w_posy + y);

}

void WindowGLFW::maximize()
{
	glfwMaximizeWindow(m_window);
	m_props->mode = WINDOW_MODE_WINDOWED_FULLSCREEN;
}

void WindowGLFW::minimize()
{
	glfwIconifyWindow(m_window);
	m_props->mode = WINDOW_MODE_MINIMIZED;
}

void WindowGLFW::restore()
{
	glfwRestoreWindow(m_window);
}

bool WindowGLFW::is_minimized() const noexcept
{
	return m_props->mode == WINDOW_MODE_MINIMIZED;
}

bool WindowGLFW::is_maximized() const noexcept
{
	return m_props->mode != WINDOW_MODE_MINIMIZED;
}

void WindowGLFW::set_window_mode(WINDOW_MODE mode)
{
	switch (mode)
	{
	case WINDOW_MODE_MINIMIZED:
		minimize();
		break;
	case WINDOW_MODE_WINDOWED:
		maximize();
		break;
	}
}

void WindowGLFW::destroy()
{
	glfwDestroyWindow(m_window);
}

void WindowGLFW::hide()
{
	glfwHideWindow(m_window);
}

void WindowGLFW::show()
{
	glfwShowWindow(m_window);
}
