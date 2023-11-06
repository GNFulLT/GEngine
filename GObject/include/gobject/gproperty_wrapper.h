#ifndef GPROPERTY_WRAPPER_H
#define GPROPERTY_WRAPPER_H

#include "gobject/gproperty.h"
#include "gobject/gobject_db.h"
#include <expected>

template<typename Class,typename Type>
class GPropertyRaw : public GProperty
{
public:
	GPropertyRaw(Type Class::*propertyPtr,const char* name) : GProperty(name, GObjectDB::get_object_db().get_type_info(typeid(Type).hash_code()),
		GObjectDB::get_object_db().get_type_info(typeid(Class).hash_code()))
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