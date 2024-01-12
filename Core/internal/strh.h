#ifndef STRH_H
#define STRH_H

#include <vector>
#include "public/core/string/gchar.h"
#include "public/core/string/os_str.h"
//
////---------------------------------------------------- Binary Search
//template<typename CharType>
//bool XID_START_CONTAINS(CharType chr);
//
//template<typename CharType>
//bool XID_CONTINUE_CONTAINS(CharType chr);



template<typename CharType>
inline void create_gstr(const CharType* src, std::vector<GCharT::type>& dst)
{
	auto len = PlatformTypes::strlen<CharType>(src); // Null - terminated
	auto dstLen = GEngine::Core::API::get_converted_length((const CharType*)src, (const GCharT::type*)nullptr) + 1;
	dst.resize(dstLen);
	GEngine::Core::API::convert_str(src,dst, dstLen);
}

template<typename CharType>
inline void convert_gstr_to_str(const CharType* src, std::string& dst)
{
	auto len = PlatformTypes::strlen<CharType>(src); // Null - terminated
	auto dstLen = GEngine::Core::API::get_converted_length((const CharType*)src, (const PlatformTypes::_ASCII*)nullptr) + 1;
	std::vector<char> arr(dstLen);
	GEngine::Core::API::convert_str(src, arr, dstLen);
	dst = std::string(arr.data());
}

template<typename CharType>
inline void create_gstr(const CharType* src, std::vector<GCharT::type>& dst,uint32_t customSize)
{
	auto len = PlatformTypes::strlen<CharType>(src); // Null - terminated
	auto dstLen = GEngine::Core::API::get_converted_length((const CharType*)src, (const GCharT::type*)nullptr) + 1;
	dst.resize(customSize);
	GEngine::Core::API::convert_str(src, dst, dstLen);
}

#endif // STRH_H