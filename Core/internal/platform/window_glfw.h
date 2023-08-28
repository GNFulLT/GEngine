#ifndef WINDOW_GLFW_H
#define WINDOW_GLFW_H

#include "public/platform/window.h"

//X GLFW 

#include <GLFW/glfw3.h>

class WindowGLFW : public Window
{
public:
	WindowGLFW(const GSharedPtr<WindowProps> props) : Window(props)
	{}

	virtual ~WindowGLFW() = default;

	//X Resizes the window by given x y
	virtual void resize(uint32_t x, uint32_t y) override;

	//X Move window to the X, Y coordinate system
	virtual void move_to(uint32_t x, uint32_t y) override;

	//X Maximize / Minimize / Restore
	virtual void maximize() override;

	virtual void minimize() override;

	virtual void restore() override;

	virtual bool is_minimized() const noexcept override;

	virtual bool is_maximized() const noexcept override;

	virtual void set_window_mode(WINDOW_MODE mode) override;

	virtual void destroy() override;

	virtual void hide() override;

	virtual void show() override;

	virtual WINDOW_MODE get_window_mode() const noexcept override;

	virtual bool is_visible() const noexcept override;

	virtual void* get_native_handler() const noexcept override;

	virtual uint32_t init() override;

	virtual void pump_messages() override;

	virtual const WindowProps& get_window_props() const noexcept;
	
	virtual bool wants_close() const override;

	static Window* create_default_window();
private:
	GLFWwindow* m_window = nullptr;
	GLFWmonitor* m_monitor = nullptr;



private:
	//X CALLBACKS


	void on_resize(int width, int height);
	void on_iconify_changed(int iconified);
	void on_move(int x, int y);
	void on_maximized(int maximized);

};

#endif // WINDOW_GLFW_H