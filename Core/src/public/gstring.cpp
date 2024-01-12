#include "public/core/string/gstring.h"

#include "public/typedefs.h"

#include "internal/strh.h"
//X String helper
#include "public/platform/window.h"



GString::~GString()
{
	int a = 5;
}

GString::GString(const PlatformTypes::_ASCII* pStr)
{
	create_gstr(pStr, this->m_data);
}

GString::GString(const PlatformTypes::_WCHAR* pStr)
{
	create_gstr(pStr, this->m_data);
}

std::string GString::convert_gstring_to_string(const GString& str)
{
	std::string dst;
	convert_gstr_to_str(str.get_data(), dst);
	return dst;
}

GString::GString(const PlatformTypes::_ASCII* pStr, uint32_t extendAlloc)
{
	create_gstr(pStr, this->m_data,extendAlloc);
}

GString::GString(const PlatformTypes::_WCHAR* pStr, uint32_t extendAlloc)
{
	create_gstr(pStr, this->m_data, extendAlloc);
}

const GCharF* GString::get_data() const noexcept
{
	return m_data.size() == 0 ? nullptr : &m_data[0];
}

GCharF* GString::get_data_mutable() noexcept
{
	return m_data.size() == 0 ? nullptr : &m_data[0];
}

uint32_t GString::get_length() const noexcept
{
	return m_data.size();
}

bool GString::is_pure_ascii_127() const noexcept
{
	return sizeof(GCharF) == 4;
}

bool GString::is_pure_ascii_255() const noexcept
{
	return std::is_unsigned<GCharF>();
}
