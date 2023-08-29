#include "internal/platform/mouse_glfw.h"

GLFWMouseManager::GLFWMouseManager(GLFWwindow* f)
{
	m_window = f;
	m_isLeftButtonClicking = false;
	m_isMiddleButtonClicking = false;
	m_isRightButtonClicking = false;


	m_isFirstClickLeftButton = false;
	m_isFirstClickRightButton = false;
	m_isFirstClickMiddleButton = false;
}

IMouseManager::on_mouse_move_slot::signal_ref GLFWMouseManager::bind_on_mouse_move_event(on_mouse_move_callback_func func)
{
	return m_slot.connect(func);
}

bool GLFWMouseManager::get_mouse_button_state(MOUSE_BUTTON btn)
{
	switch (btn)
	{
	case MOUSE_BUTTON_LEFT:
		return m_isLeftButtonClicking;
	case MOUSE_BUTTON_RIGHT:
		return m_isRightButtonClicking;
	case MOUSE_BUTTON_MIDDLE:
		return m_isMiddleButtonClicking;
	default:
		return false;
	}
}

std::pair<int, int> GLFWMouseManager::get_mouse_pos()
{
	double xpos, ypos;
	glfwGetCursorPos(m_window,&xpos,&ypos);
	return { int(xpos),int(ypos) };
}

void GLFWMouseManager::on_mouse_move(int x, int y)
{
	m_slot(x, y);
}

void GLFWMouseManager::on_mouse_click(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			m_isRightButtonClicking = true;
		}
		else if (action == GLFW_RELEASE)
		{
			m_isRightButtonClicking = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			
			m_isLeftButtonClicking = true;
		}
		else if (action == GLFW_RELEASE)
		{
			m_isLeftButtonClicking = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (action == GLFW_PRESS)
		{
			m_isMiddleButtonClicking = true;
		}
		else if (action == GLFW_RELEASE)
		{
			m_isMiddleButtonClicking = false;
		}
	}
}
