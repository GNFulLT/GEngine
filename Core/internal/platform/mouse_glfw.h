#ifndef MOUSE_GLFW_H
#define MOUSE_GLFW_H

#include "public/platform/imouse_manager.h"

#include <GLFW/glfw3.h>

class WindowGLFW;

class GLFWMouseManager : public IMouseManager
{
	friend class WindowGLFW;
public:
	GLFWMouseManager(GLFWwindow* f);

	// Inherited via IMouseManager
	virtual on_mouse_move_slot::signal_ref bind_on_mouse_move_event(on_mouse_move_callback_func func) override;

	virtual bool get_mouse_button_state(MOUSE_BUTTON btn) override;

	virtual std::pair<int, int> get_mouse_pos() override;

protected:
	void on_mouse_move(int x, int y);
	void on_mouse_click(int button, int action, int mods);
private:

	GLFWwindow* m_window;
	on_mouse_move_slot m_slot;

	bool m_isFirstClickLeftButton;
	bool m_isFirstClickRightButton;
	bool m_isFirstClickMiddleButton;

	bool m_isLeftButtonClicking;
	bool m_isRightButtonClicking;
	bool m_isMiddleButtonClicking;
};

#endif // MOUSE_GLFW_H