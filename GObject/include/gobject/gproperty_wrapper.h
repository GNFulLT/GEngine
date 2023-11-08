#ifndef GPROPERTY_WRAPPER_H
#define GPROPERTY_WRAPPER_H

#include "gobject/gobject_db.h"
#include "gobject/gproperty_errors.h"
#include <expected>
#include "gobject/gvariant.h"
#include "gobject/gtype_utils.h"

class GProperty;


class GPropertyWrapper
{

public:
	friend class GProperty;
	friend class GObjectDB;
	//X Getter Setter
	virtual std::expected<void, GPROPERTY_SET_ERROR> set(GVariant object, GVariant value) = 0;
	virtual std::expected<GVariant, GPROPERTY_GET_ERROR> get(GVariant object) = 0;

protected:
	GPropertyWrapper(const char* name, const GTypeInfo* typeInfo, const GTypeInfo* parentInfo)
	{
		m_name = name;
		m_typeInfo = create_type(typeInfo);
		m_parentInfo = create_type(parentInfo);
	}
	std::string m_name;
	GType m_typeInfo;
	GType m_parentInfo;
};

template<typename Class, typename Type>
class GPropertyGetterSetter : public GPropertyWrapper
{
public:
	using setter_func = void(Class::*)(const Type&);
	using getter_func = const Type&(Class::*)();

	GPropertyGetterSetter(Type Class::* propertyPtr, const char* name, getter_func getter = nullptr, setter_func setter = nullptr) : GPropertyWrapper(name, GTypeUtils::add_or_get_type_info<Type>(),
		GTypeUtils::add_or_get_type_info<Class>())
	{
		mPropertyPtr = propertyPtr;
		mGetter = getter;
		mSetter = setter;
	}

	virtual std::expected<void, GPROPERTY_SET_ERROR> set(GVariant object, GVariant value) override
	{
		if (!object.get_type().equals(m_parentInfo))
		{
			return std::unexpected(GPROPERTY_SET_ERROR_INVALID_OBJECT);
		}
		set_impl(object, value, std::is_const<Type>());
	}

	virtual std::expected<GVariant, GPROPERTY_GET_ERROR> get(GVariant object) override
	{
		if (!object.get_type().equals(m_parentInfo))
		{
			return std::unexpected(GPROPERTY_GET_ERROR_INVALID_OBJECT);
		}
		//X Unsafe cast
		Class* obj = ((Class*)object.get_raw());
		return GVariant((obj->*mGetter)());
	}

private:
	std::expected<void, GPROPERTY_SET_ERROR> set_impl(GVariant object, GVariant value, std::false_type)
	{
		if (!value.get_type().equals(m_typeInfo))
		{
			return std::unexpected(GPROPERTY_SET_ERROR_INVALID_PROPERTY_TYPE);
		}
		Class* obj = ((Class*)object.get_raw());
		Type* castedType = (Type*)value.get_raw();
		(obj->*mSetter)(*castedType);
		return std::expected<void, GPROPERTY_SET_ERROR>();
	}

	std::expected<GVariant, GPROPERTY_SET_ERROR> set_impl(GVariant object, const GVariant value, std::true_type)
	{
		return std::unexpected(GPROPERTY_SET_ERROR_PROPERTY_WAS_NATIVE_CONST);
	}
private:
	Type Class::* mPropertyPtr;
	getter_func mGetter;
	setter_func mSetter;
};

template<typename Class,typename Type>
class GPropertyRaw : public GPropertyWrapper
{
public:
	GPropertyRaw(Type Class::*propertyPtr,const char* name) : GPropertyWrapper(name, GTypeUtils::add_or_get_type_info<Type>(),
		GTypeUtils::add_or_get_type_info<Class>())
	{
		mPropertyPtr = propertyPtr;
	}


	virtual std::expected<void,GPROPERTY_SET_ERROR> set(GVariant object, GVariant value) override
	{
		if (!object.get_type().equals(m_parentInfo))
		{
			return std::unexpected(GPROPERTY_SET_ERROR_INVALID_OBJECT);
		}
		set_impl(object, value, std::is_const<Type>());
	}

	virtual std::expected<GVariant, GPROPERTY_GET_ERROR> get(GVariant object) override
	{
		if (!object.get_type().equals(m_parentInfo))
		{
			return std::unexpected(GPROPERTY_GET_ERROR_INVALID_OBJECT);
		}
		//X Unsafe cast
		return ((Class*)object.get_raw())->*mPropertyPtr;
	}

private:
	std::expected<void, GPROPERTY_SET_ERROR> set_impl(GVariant object, GVariant value, std::false_type)
	{
		if (!value.get_type().equals(m_typeInfo))
		{
			return std::unexpected(GPROPERTY_SET_ERROR_INVALID_PROPERTY_TYPE);
		}
		Class* obj = ((Class*)object.get_raw());
		Type* castedType = (Type*)value.get_raw();
		obj->*mPropertyPtr = *castedType;
		return std::expected<void, GPROPERTY_SET_ERROR>();
	}

	std::expected<GVariant, GPROPERTY_SET_ERROR> set_impl(GVariant object, const GVariant value, std::true_type)
	{
		return std::unexpected(GPROPERTY_SET_ERROR_PROPERTY_WAS_NATIVE_CONST);
	}
private:
	Type Class::* mPropertyPtr;
};

#endif // GPROPERTY_WRAPPER_H