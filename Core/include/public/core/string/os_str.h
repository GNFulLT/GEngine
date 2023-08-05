#ifndef OSSTR_H
#define OSSTR_H

#include "public/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include "public/core/string/gchar.h"
#include <vector>

namespace GEngine::Core::API
{

	//X TO TCHAR
	CORE_API uint32_t get_converted_length(_In_ const PlatformTypes::_ASCII* src,_In_ const PlatformTypes::_WCHAR* dst) noexcept;
	//X FROM TCHAR
	CORE_API uint32_t get_converted_length(const PlatformTypes::_WCHAR* src, const PlatformTypes::_ASCII* dst) noexcept;

	CORE_API uint32_t get_converted_length(const PlatformTypes::_WCHAR* src, const PlatformTypes::_WCHAR* dst) noexcept;
	CORE_API uint32_t get_converted_length(const PlatformTypes::_ASCII* src, const PlatformTypes::_ASCII* dst) noexcept;



	//X TO TCHAR
	CORE_API void convert_str(_In_ const PlatformTypes::_ASCII* src, _Inout_ std::vector<GCharWide::type>& dst,_In_ uint32_t srcLen) noexcept;
	//X FROM TCHAR
	CORE_API void convert_str(_In_ const PlatformTypes::_WCHAR* src, _Inout_ std::vector<GCharA::type>& dst, _In_ uint32_t srcLen) noexcept;

	CORE_API void convert_str(_In_ const PlatformTypes::_WCHAR* src, _Inout_ std::vector<GCharWide::type>& dst, _In_ uint32_t srcLen) noexcept;
	CORE_API void convert_str(_In_ const PlatformTypes::_ASCII* src, _Inout_ std::vector<GCharA::type>& dst, _In_ uint32_t srcLen) noexcept;


}

#endif // OSSTR_H