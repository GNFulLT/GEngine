#include "public/globals.h"

bool g_exitRequested = false;

CORE_API void request_exit()
{
	g_exitRequested = true;
}