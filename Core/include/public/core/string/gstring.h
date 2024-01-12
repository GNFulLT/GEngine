#ifndef GSTRING_H
#define GSTRING_H

#include "public/core/string/gchar.h"
#include "public/GEngine_EXPORT.h"

#include <vector>
#include <string_view>
#include <string>

class CORE_API GString
{
#pragma message("ADD CUSTOM ALLOCATOR TO VECTOR GString")
	//X Needs to add custom allocator
	typedef std::vector<GCharF> CharArray;

	CharArray m_data;

public:
	
	static std::string convert_gstring_to_string(const GString& str);

	//X Defaults
	GString(void) = default;
	GString(GString&&) = default;
	GString(const GString&) = default;
	GString& operator=(const GString&) = default;

	~GString();
	//X Ctors

	GString(const PlatformTypes::_ASCII* pStr);
	GString(const PlatformTypes::_WCHAR* pStr);

	GString(const PlatformTypes::_ASCII* pStr,uint32_t extendAlloc);
	GString(const PlatformTypes::_WCHAR* pStr, uint32_t extendAlloc);

	//X Methods
	const GCharF* get_data() const noexcept;
	GCharF* get_data_mutable() noexcept;
	uint32_t get_length() const noexcept;
	bool is_pure_ascii_127() const noexcept;
	bool is_pure_ascii_255() const noexcept;

};

#endif // GSTRING_H