#ifndef GLOBALS_H
#define GLOBALS_H

#include "GEngine_EXPORT.h"
#include "public/typedefs.h"

extern ENGINE_API bool g_exitRequested;

extern ENGINE_API bool g_updateEnable;

_F_INLINE_ bool get_exited()
{
	return g_exitRequested;
}

_F_INLINE_ bool get_update_enabled()
{
	return g_updateEnable;
}
ENGINE_API void request_exit();

ENGINE_API void enable_update();

ENGINE_API void disable_update();

#endif // GLOBALS_h