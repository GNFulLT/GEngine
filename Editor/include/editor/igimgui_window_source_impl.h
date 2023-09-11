#ifndef IGIMGUI_WINDOW_SOURCE_IMPL_H
#define IGIMGUI_WINDOW_SOURCE_IMPL_H

#include <cstdint>
#include "GEngine_EXPORT.h"
#include "editor/igimgui_window_impl.h"


class EDITOR_API IGImGuiWindowSourceImpl
{
public:
	virtual ~IGImGuiWindowSourceImpl() = default;
	virtual IGImGuiWindowImpl* create_window() = 0;

	
private:
};

#endif // IGIMGUI_WINDOW_SOURCE_H