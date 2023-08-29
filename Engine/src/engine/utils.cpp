#include "internal/engine/utils.h"
#include <cstring>

int endsWith(const char* s, const char* part)
{
	return (strstr(s, part) - s) == (strlen(s) - strlen(part));
}
