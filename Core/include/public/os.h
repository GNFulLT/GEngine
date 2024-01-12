#ifndef OS_H
#define OS_H

#pragma message("NOT SURE WCTYPE SHOULD BE INCLUDED HERE GCHAR")
#include <wctype.h>
#include <cstddef>
#include <cctype>
#include <cstdint>
#include <string>

#ifdef WIN32
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#else
#error PLATFORM NOT SPECIFIED
#endif

namespace PlatformTypes
{
#ifdef PLATFORM_WINDOWS
	/*
	* Platform dependent char type
	* it
	*/
	typedef wchar_t _TCHAR;
	typedef wchar_t _WCHAR;
	typedef wint_t _WINT;
	typedef char _ASCII;
#define _T(str) L##str
#elif PLATFORM_LINUX
	typedef wchar_t _WCHAR;
	typedef char _TCHAR;
	typedef char _WINT;
	typedef char _ASCII;
#define _T(str) str
#else 
#error PLATFORM NOT SPECIFIED
#endif

	template<typename CharType>
	inline uint32_t strlen(const CharType* src);

	template<>
	inline uint32_t strlen(const _ASCII* src)
	{
		return (uint32_t)::strlen(src);
	}

	template<>
	inline uint32_t strlen(const _WCHAR* src)
	{
		return (uint32_t)::wcslen(src);
	}

}


#endif // OS_H