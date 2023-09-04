#ifndef WINDOW_GLFW_H
#define WINDOW_GLFW_H

#include "public/platform/window.h"
#include <memory>
#include "internal/platform/mouse_glfw.h"
#include "internal/platform/keyboard_glfw.h"

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

	virtual void move_by(uint32_t x, uint32_t y) override;

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

	GLFWMouseManager* get_glfw_mouse();

	virtual IMouseManager* get_mouse_manager() const override;

	static Window* create_default_window();
private:
	GLFWwindow* m_window = nullptr;
	GLFWmonitor* m_monitor = nullptr;
	std::unique_ptr<GLFWMouseManager> m_mouseManager;
	std::unique_ptr<GLFWKeyboardManager> m_keyboardManager;

private:
	//X CALLBACKS


	void on_resize(int width, int height);
	void on_iconify_changed(int iconified);
	void on_move(int x, int y);
	void on_maximized(int maximized);
	void on_mouse_move(int x, int y);
	void on_mouse_click(int button,int action,int mods);
	void on_key(int key, int scancode, int action, int code);

	// Inherited via Window
	virtual IKeyboardManager* get_keyboard_manager() const override;
};

#endif // WINDOW_GLFW_H