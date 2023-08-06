#ifndef GSTRING_UTILS_H
#define GSTRING_UTILS_H

#include <string>
#include "public/typedefs.h"

_INLINE_ std::string string32_to_string(const std::u32string& str)
{
    std::string out;
    // Have same char
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] <= 0x7F) {
            out += (char)str[i];
        }
        else if (str[i] <= 0x7FF) {
            out += 0xC0 | (char)((str[i] >> 6) & 0x1F);
            out += 0x80 | (char)(str[i] & 0x3F);
        }
        else if (str[i] <= 0xFFFF) {
            out += 0xE0 | (char)((str[i] >> 12) & 0x0F);
            out += 0x80 | (char)((str[i] >> 6) & 0x3F);
            out += 0x80 | (char)(str[i] & 0x3F);
        }
        else if (str[i] <= 0x10FFFF) {
            out += 0xF0 | (char)((str[i] >> 18) & 0x0F);
            out += 0x80 | (char)((str[i] >> 12) & 0x3F);
            out += 0x80 | (char)((str[i] >> 6) & 0x3F);
            out += 0x80 | (char)(str[i] & 0x3F);
        }
        else {
            throw std::runtime_error("Invalid UTF-32 String");
        }
    }
    return out;
}

_INLINE_ std::vector<std::string> split_string(const std::string& str, const char delim) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    while (pos < str.size()) {
        size_t end_pos = str.find(delim, pos);
        if (end_pos == std::string::npos) {
            end_pos = str.size();
        }
        tokens.push_back(str.substr(pos, end_pos - pos));
        pos = end_pos + 1;
    }
    return tokens;
}


// Source : https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time
namespace impl
{
    template <std::string_view const&, typename, std::string_view const&, typename>
    struct concat;

    template <std::string_view const& S1,
        std::size_t... I1,
        std::string_view const& S2,
        std::size_t... I2>
    struct concat<S1, std::index_sequence<I1...>, S2, std::index_sequence<I2...>>
    {
        static constexpr const char value[]{ S1[I1]..., S2[I2]..., 0 };
    };
} // namespace impl
template <std::string_view const&...> struct join;

template <>
struct join<>
{
    static constexpr std::string_view value = "";
};

template <std::string_view const& S1, std::string_view const& S2>
struct join<S1, S2>
{
    static constexpr std::string_view value =
        impl::concat<S1,
        std::make_index_sequence<S1.size()>,
        S2,
        std::make_index_sequence<S2.size()>>::value;
};

template <std::string_view const& S, std::string_view const&... Rest>
struct join<S, Rest...>
{
    static constexpr std::string_view value =
        join<S, join<Rest...>::value>::value;
};

template <std::string_view const&... Strs>
static constexpr auto join_v = join<Strs...>::value;

#endif // GSTRING_UTILS_H