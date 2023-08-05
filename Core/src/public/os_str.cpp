#include "public/core/string/os_str.h"
#include "public/core/templates/shared_ptr.h"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <stringapiset.h>

#endif

namespace GEngine::Core::API
{ 

CORE_API uint32_t get_converted_length(const PlatformTypes::_ASCII* src, const PlatformTypes::_WCHAR* dst) noexcept
{
#ifdef PLATFORM_WINDOWS
	return MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,src,-1,(LPWSTR)dst,0);
#elif defined(PLATFORM_LINUX)
#error Needed Platform specific define
#else
#error Needed Platform specific define
#endif
}

CORE_API uint32_t get_converted_length(const PlatformTypes::_WCHAR * src, const PlatformTypes::_ASCII* dst) noexcept
{
#ifdef PLATFORM_WINDOWS
	return WideCharToMultiByte(CP_UTF8, 0, src, -1, (LPSTR)dst, 0, 0, 0);
#elif defined(PLATFORM_LINUX)
#error Needed Platform specific define
#else
#error Needed Platform specific define
#endif
}




CORE_API uint32_t get_converted_length(const PlatformTypes::_WCHAR* src, const PlatformTypes::_WCHAR* dst) noexcept
{
	return ::wcslen(src);
}
CORE_API uint32_t get_converted_length(const PlatformTypes::_ASCII* src, const PlatformTypes::_ASCII* dst) noexcept
{
	return ::strlen(src);
}

CORE_API void convert_str(_In_ const PlatformTypes::_ASCII* src, _Inout_ std::vector<GCharWide::type>& dst,_In_ uint32_t srcLen) noexcept
{
#ifdef PLATFORM_WINDOWS
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, (LPWSTR)&dst[0], srcLen);
	dst.push_back('\0');
#elif defined(PLATFORM_LINUX)
#error Needed Platform specific define
#else
#error Needed Platform specific define
#endif
}

CORE_API void convert_str(_In_ const PlatformTypes::_WCHAR* src, _Inout_ std::vector<GCharA::type>& dst, _In_ uint32_t srcLen) noexcept
{
#ifdef PLATFORM_WINDOWS
	WideCharToMultiByte(CP_UTF8, 0, src, -1, (LPSTR)&dst[0], srcLen, 0, 0);
	dst.push_back('\0');
#elif defined(PLATFORM_LINUX)
#error Needed Platform specific define
#else
#error Needed Platform specific define
#endif
}

CORE_API void convert_str(_In_ const PlatformTypes::_WCHAR* src, _Inout_ std::vector<GCharWide::type>& dst, _In_ uint32_t srcLen) noexcept
{
	wcscpy(&dst[0], src);
}

CORE_API void convert_str(_In_ const PlatformTypes::_ASCII* src, _Inout_ std::vector<GCharA::type>& dst, _In_ uint32_t srcLen) noexcept
{
	strcpy(&dst[0], src);
}

}


