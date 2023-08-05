#ifndef GLOBALS_H
#define GLOBALS_H

#include "GEngine_EXPORT.h"
#include "public/typedefs.h"

extern ENGINE_API bool g_exitRequested;

_F_INLINE_ bool get_exited()
{
	return g_exitRequested;
}

ENGINE_API void request_exit();

#endif // GLOBALS_h