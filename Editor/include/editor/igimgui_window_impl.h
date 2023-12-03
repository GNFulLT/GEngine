#ifndef IIMGUI_WINDOW_IMPL_H
#define IIMGUI_WINDOW_IMPL_H

#include <cstdint>
#include "GEngine_EXPORT.h"

enum GIMGUIWINDOWDIR
{
	GIMGUIWINDOWDIR_NONE,
	GIMGUIWINDOWDIR_LEFT,
	GIMGUIWINDOWDIR_RIGHT,
	GIMGUIWINDOWDIR_MIDDLE,
	GIMGUIWINDOWDIR_BOTTOM,
	GIMGUIWINDOWDIR_LEFT_BOTTOM,
	GIMGUIWINDOWDIR_RIGHT_BOTTOM
};

struct GImGuiWindowStorage
{
	int width;
	int height;
};


class EDITOR_API IGImGuiWindowImpl
{
public:
	virtual ~IGImGuiWindowImpl() = default;

	//X If it returns false. The window will be discarded
	virtual bool init() = 0;

	virtual void set_storage(GImGuiWindowStorage* storage) = 0;

	virtual bool need_render() = 0;
	virtual void render() = 0;
	virtual void on_resize() = 0;
	
	//X TODO : This will be called in update method maybe.
	virtual void on_data_update() = 0;

	virtual void destroy() = 0;

	virtual const char* get_window_name() = 0;

	virtual const char* get_window_id() { return nullptr; };

	virtual int get_flag() { return 0; }

	virtual bool can_open_multiple() const { return false; };
private: 
};

#endif // IIMGUI_WINDOW_H