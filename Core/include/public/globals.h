#ifndef GLOBALS_H
#define GLOBALS_H

#include "GEngine_EXPORT.h"
#include "typedefs.h"

extern CORE_API bool g_exitRequested;

_F_INLINE_ bool get_exited()
{
	return g_exitRequested;
} 

CORE_API void request_exit();

#endif // GLOBALS_h