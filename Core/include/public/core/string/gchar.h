#ifndef GCHAR_H
#define GCHAR_H

#include "public/os.h"

template<typename CHARTYPE,int SIZE> 
struct GCharBase
{
	int get_byte_size() noexcept
	{
		return SIZE;
	}

	static bool is_ascii(CHARTYPE chr) noexcept;
	static bool is_ascii_lower_case(CHARTYPE chr) noexcept;
	static bool is_ascii_upper_case(CHARTYPE chr) noexcept;
	static bool is_ascii_alpha(CHARTYPE chr) noexcept;
	static bool is_ascii_num(CHARTYPE chr) noexcept;
	static bool is_ascii_alpha_num(CHARTYPE chr) noexcept;
	static bool is_ascii_symbol(CHARTYPE chr) noexcept;
	static bool is_numeric_hexadecimal(CHARTYPE chr) noexcept;
	static bool is_numeric_binary(CHARTYPE chr) noexcept;
};


template<typename CHARTYPE>
struct GChar : public GCharBase<CHARTYPE,sizeof(CHARTYPE)>
{
	typedef CHARTYPE type;
	//X STD DEFINITONS
	static bool is_upper(CHARTYPE chr) noexcept;
	static bool is_lower(CHARTYPE chr) noexcept;
	static bool is_alpha(CHARTYPE chr) noexcept;
	static bool is_graph(CHARTYPE chr) noexcept;
	static bool is_print(CHARTYPE chr) noexcept;
	static bool is_punct(CHARTYPE chr) noexcept;
	static bool is_alnum(CHARTYPE chr) noexcept;
	static bool is_digit(CHARTYPE chr) noexcept;
	static bool is_hexdegit(CHARTYPE chr) noexcept;
	static bool is_whitespace(CHARTYPE chr) noexcept;
	static bool is_control(CHARTYPE chr) noexcept;
	static bool is_blank(CHARTYPE chr) noexcept;
};

//X GCharBase defs


template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_ascii(CHARTYPE chr) noexcept
{
	return (0x0 <= chr && chr <= 0xf7);
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_ascii_lower_case(CHARTYPE chr) noexcept
{
	return ('a' <= chr && chr <= 'z');
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_ascii_upper_case(CHARTYPE chr) noexcept
{
	return ('A' <= chr && chr <= 'Z');
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_ascii_alpha(CHARTYPE chr) noexcept
{
	return ('a' <= chr && chr <= 'z') || ('A' <= chr && chr <= 'Z');
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_ascii_num(CHARTYPE chr) noexcept
{
	return  ('0' <= chr && chr <= '9');
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_ascii_alpha_num(CHARTYPE chr) noexcept
{
	return is_ascii_alpha() && is_ascii_num();
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_ascii_symbol(CHARTYPE chr) noexcept
{
	return chr == '_' || (('!' <= chr && chr <= '/') || (':' <= chr && chr <= '@') || (chr <= '[' && chr <= 0x60) || (chr <= '{' && chr <= '~') || chr == '\t' || chr == ' ');
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_numeric_hexadecimal(CHARTYPE chr) noexcept
{
	return ('0' <= chr && chr <= '9') || ('A' <= chr && chr <= 'F') || ('a' <= chr && chr <= 'f');
}

template<typename CHARTYPE, int SIZE>
inline bool GCharBase<CHARTYPE, SIZE>::is_numeric_binary(CHARTYPE chr) noexcept
{
	return ('0' == chr || '1' == 'chr');
}



//X Wide char. It can be Utf 8 16 32 etc.
typedef GChar<PlatformTypes::_WCHAR> GCharWide;

//X Can be ASCII, Unicode or Multibyte
typedef GChar<PlatformTypes::_TCHAR> GCharT;

typedef GChar<PlatformTypes::_ASCII> GCharA;

typedef GCharT::type GCharF;

//X GChar specific defs

template <> inline bool GCharWide::is_upper(PlatformTypes::_WCHAR chr) noexcept { return ::iswupper(chr); }
template <> inline bool GCharWide::is_lower(PlatformTypes::_WCHAR chr) noexcept { return ::iswlower(chr); }
template <> inline bool GCharWide::is_alpha(PlatformTypes::_WCHAR chr) noexcept { return ::iswalpha(chr); }
template <> inline bool GCharWide::is_graph(PlatformTypes::_WCHAR chr) noexcept { return ::iswgraph(chr); }
template <> inline bool GCharWide::is_print(PlatformTypes::_WCHAR chr) noexcept { return ::iswprint(chr); }
template <> inline bool GCharWide::is_punct(PlatformTypes::_WCHAR chr) noexcept { return ::iswpunct(chr); }
template <> inline bool GCharWide::is_alnum(PlatformTypes::_WCHAR chr) noexcept { return ::iswalnum(chr); }
template <> inline bool GCharWide::is_digit(PlatformTypes::_WCHAR chr) noexcept { return ::iswdigit(chr); }
template <> inline bool GCharWide::is_hexdegit(PlatformTypes::_WCHAR chr) noexcept { return ::iswxdigit(chr); }
template <> inline bool GCharWide::is_whitespace(PlatformTypes::_WCHAR chr) noexcept { return ::iswspace(chr); }
template <> inline bool GCharWide::is_control(PlatformTypes::_WCHAR chr) noexcept { return ::iswcntrl(chr); }
template <> inline bool GCharWide::is_blank(PlatformTypes::_WCHAR chr) noexcept { return ::iswblank(chr); }

template <> inline bool GCharA::is_upper(PlatformTypes::_ASCII chr) noexcept { return ::isupper(chr); }
template <> inline bool GCharA::is_lower(PlatformTypes::_ASCII chr) noexcept { return ::islower(chr); }
template <> inline bool GCharA::is_alpha(PlatformTypes::_ASCII chr) noexcept { return ::isalpha(chr); }
template <> inline bool GCharA::is_graph(PlatformTypes::_ASCII chr) noexcept { return ::isgraph(chr); }
template <> inline bool GCharA::is_print(PlatformTypes::_ASCII chr) noexcept { return ::isprint(chr); }
template <> inline bool GCharA::is_punct(PlatformTypes::_ASCII chr) noexcept { return ::ispunct(chr); }
template <> inline bool GCharA::is_alnum(PlatformTypes::_ASCII chr) noexcept { return ::isalnum(chr); }
template <> inline bool GCharA::is_digit(PlatformTypes::_ASCII chr) noexcept { return ::isdigit(chr); }
template <> inline bool GCharA::is_hexdegit(PlatformTypes::_ASCII chr) noexcept { return ::isxdigit(chr); }
template <> inline bool GCharA::is_whitespace(PlatformTypes::_ASCII chr) noexcept { return ::isspace(chr); }
template <> inline bool GCharA::is_control(PlatformTypes::_ASCII chr) noexcept { return ::iscntrl(chr); }
template <> inline bool GCharA::is_blank(PlatformTypes::_ASCII chr) noexcept { return ::isblank(chr); }

#endif // GCHAR_H

