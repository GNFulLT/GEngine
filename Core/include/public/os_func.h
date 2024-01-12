#ifndef OS_FUNC_H
#define OS_FUNC_H

#include <public/GEngine_EXPORT.h>
#include <expected>
#include "public/core/string/gstring.h"

namespace PlatformFunctions
{
	CORE_API std::expected<int, GString> execute_shell_command(const GString& str);
}

#endif // OS_FUNC_H