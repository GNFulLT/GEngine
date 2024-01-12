#include "public/os_func.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <comdef.h>
#elif
#error IMPLEMENT
#endif


std::expected<int, GString> PlatformFunctions::execute_shell_command(const GString& str)
{
	auto data = GString::convert_gstring_to_string(str);
	HINSTANCE shellResult = ShellExecuteA(NULL, "open","cmd.exe", data.c_str(), NULL, SW_SHOWNORMAL);
	if (reinterpret_cast<int>(shellResult) > 32) {
		return 0;
	}
	DWORD lastError = GetLastError();
	HRESULT lastErrAsHRESULT = HRESULT_FROM_WIN32(lastError);
	_com_error error(lastErrAsHRESULT);
	LPCTSTR errorText = error.ErrorMessage();
	GString asStr(errorText);

	return std::unexpected(asStr);
}
