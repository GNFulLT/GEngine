#include "engine/globals.h"

bool g_exitRequested = false;

ENGINE_API void request_exit()
{
	g_exitRequested = true;
}
