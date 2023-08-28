#ifndef WINDOW_PROPS_H
#define WINDOW_PROPS_H

#include <cstdint>
#include <string>

enum WINDOW_MODE : uint8_t
{
	WINDOW_MODE_WINDOWED = 0,
	WINDOW_MODE_WINDOWED_FULLSCREEN,
	WINDOW_MODE_FULLSCREEN,
	WINDOW_MODE_MINIMIZED
};

struct WindowProps
{
	uint32_t width;
	uint32_t height;
	uint32_t posx;
	uint32_t posy;
	std::string title;
	WINDOW_MODE mode = WINDOW_MODE_WINDOWED;
	bool hasCaption = true;
	bool isVisible = true;
};

#endif // WINDOW_PROPS_H