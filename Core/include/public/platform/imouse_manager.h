#ifndef IMOUSE_MANAGER_H
#define IMOUSE_MANAGER_H

#include "public/core/templates/signal/gslot.h"
#include <utility>

enum MOUSE_BUTTON
{
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE
};

class IMouseManager
{
public:
	typedef std::function<void(int, int)> on_mouse_move_callback_func;
	typedef GSlot<void(int, int)> on_mouse_move_slot;

	virtual ~IMouseManager() = default;

	virtual on_mouse_move_slot::signal_ref bind_on_mouse_move_event(on_mouse_move_callback_func func) = 0;
	
	virtual bool get_mouse_button_state(MOUSE_BUTTON btn) = 0;

	virtual std::pair<int, int> get_mouse_pos() = 0;
private:
};


#endif // IMOUSE_MANAGER_H