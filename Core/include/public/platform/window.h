#ifndef WINDOW_H
#define WINDOW_H

#include "public/GEngine_EXPORT.h"
#include "public/core/templates/shared_ptr.h"
#include "public/platform/window_props.h"

class IMouseManager;

class CORE_API Window
{
public:
	Window(const GSharedPtr<WindowProps> props) : m_props(props)
	{}

	virtual ~Window() = default;

	//X Resizes the window by given x y
	virtual void resize(uint32_t x,uint32_t y) = 0;

	//X Move window to the X, Y coordinate system
	virtual void move_to(uint32_t x, uint32_t y) = 0;

	virtual void move_by(uint32_t x, uint32_t y) = 0;


	//X Maximize / Minimize / Restore
	virtual void maximize() = 0;

	virtual void minimize() = 0;

	virtual void restore() = 0;

	virtual bool is_minimized() const noexcept = 0;
	
	virtual bool is_maximized() const noexcept = 0;

	virtual void set_window_mode(WINDOW_MODE mode) = 0;

	virtual void destroy() = 0;

	virtual void hide() = 0;

	virtual void show() = 0;

	virtual WINDOW_MODE get_window_mode() const noexcept = 0;
	
	virtual bool is_visible() const noexcept = 0;

	virtual void* get_native_handler() const noexcept = 0;

	virtual const WindowProps& get_window_props() const noexcept = 0;

	virtual uint32_t init() = 0;
	
	virtual void pump_messages() = 0;

	virtual bool wants_close() const = 0;

	virtual IMouseManager* get_mouse_manager() const = 0;

protected:
	GSharedPtr<WindowProps> m_props;
};


extern CORE_API Window* create_default_window();

extern CORE_API GSharedPtr<WindowProps> get_default_props();

extern CORE_API Window* create_with_props(GSharedPtr<WindowProps> props);
#endif // WINDOW_H