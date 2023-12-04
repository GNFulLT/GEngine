#include "engine/globals.h"

bool g_exitRequested = false;
bool g_updateEnable = true;

ENGINE_API void request_exit()
{
	g_exitRequested = true;
}

ENGINE_API void enable_update()
{
	g_updateEnable = true;
}

ENGINE_API void disable_update()
{
	g_updateEnable = false;
}
