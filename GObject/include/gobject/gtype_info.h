#ifndef GTYPE_INFO_H
#define GTYPE_INFO_H

#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"

#include <cstdint>
#include <string>
#include <bitset>
#include <vector>
#include <memory>
#include <typeinfo>
#include <sstream>
#include <cassert>
#include "public/core/templates/unordered_dense.h"
#include "gobject/gfunction_wrapper.h"
#include "gobject/gproperty_wrapper.h"

struct GClassInfo
{
    ankerl::unordered_dense::segmented_map<std::string,std::shared_ptr<GFunctionWrapper>> m_functionMap;
    ankerl::unordered_dense::segmented_map<std::string, std::shared_ptr<GPropertyWrapper>> m_propertyMap;

    std::vector<std::shared_ptr<GPropertyWrapper>> m_properties;
};

enum class TYPE_TRAIT_INFO : std::uint8_t
{
    TYPE_TRAIT_INFO_OBJECT = 0,
    TYPE_TRAIT_INFO_ENUM,
    TYPE_TRAIT_INFO_ARRAY,
    TYPE_TRAIT_INFO_POINTER,
    TYPE_TRAIT_INFO_MANAGED_POINTER,
    TYPE_TRAIT_INFO_ARITHMETIC,
    TYPE_TRAIT_INFO_FUNCTION_POINTER,
    TYPE_TRAIT_INFO_MEMBER_OBJECT_POINTER,
    TYPE_TRAIT_INFO_MEMBER_FUNCTION_POINTER,
   
    TYPE_TRAIT_INFO_COUNT
};

using TypeTraits = std::bitset<(std::size_t)TYPE_TRAIT_INFO::TYPE_TRAIT_INFO_COUNT>;

struct GOBJECT_API GTypeInfo
{
    GTypeInfo* m_wrappedType;

    std::string m_name;
    std::uint64_t m_id;
    bool m_is_valid;


    std::size_t m_size;

    TypeTraits m_traits;

    _F_INLINE_ bool is_type_trait(TYPE_TRAIT_INFO info) const _NO_EXCEPT_
    {
        return m_traits.test(static_cast<std::size_t>(info));
    }

    GClassInfo m_classInfo;

};


template<typename T>
constexpr _F_INLINE_ std::unique_ptr<GTypeInfo> create_type_info_for(typename std::enable_if<!std::is_void<T>::value>::type* = 0)
{
    const std::type_info* stdinf = &typeid(T);
    GTypeInfo* inf = new GTypeInfo();
    inf->m_is_valid = true;
    inf->m_id = stdinf->hash_code();
    inf->m_size = sizeof(T);
    
    std::istringstream  fullName(stdinf->name());

    std::vector<std::string> strings;

    std::string iter;
    while (getline(fullName, iter, ' ')) {
        strings.push_back(iter);
    }
    if (strings.size() > 1)
    {
        inf->m_name = strings[1];

        //X TODO : TYPE TRAITS

        if (strings[0] == std::string("class"))
        {

        }
        else if (strings[0] == std::string("struct"))
        {

        }
        else
        {
            //X TODO : CSTDINT NAMES ARE DIFFERENT IN DIFFERENT PLATFORMS
            
        }
    }
    else
    {
        inf->m_name = strings[0];
    }
   

    //X TODO : TYPETRAITS, WRAPPED TYPE AND CLASS DATA

    return std::unique_ptr<GTypeInfo>(inf);
    
}


// VOID CREATOR
template<typename T>
constexpr _F_INLINE_ std::unique_ptr<GTypeInfo> create_type_info_for(typename std::enable_if<std::is_void<T>::value>::type* = 0)
{
    GTypeInfo* inf = new GTypeInfo();
    inf->m_is_valid = true;
    inf->m_id = 0;
    inf->m_size = 0;

    inf->m_name = "void";

    //X TODO : TYPETRAITS, WRAPPED TYPE AND CLASS DATA

    return std::unique_ptr<GTypeInfo>(inf);

}

#endif // GTYPE_INFO_H