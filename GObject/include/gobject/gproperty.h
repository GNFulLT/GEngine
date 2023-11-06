#ifndef GPROPERTY_H
#define GPROPERTY_H

#include <string>

#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include "gobject/gvariant.h"
#include <expected>

struct GTypeInfo;

enum GPROPERTY_GET_ERROR
{
	GPROPERTY_GET_ERROR_INVALID_OBJECT
};


enum GPROPERTY_SET_ERROR
{
	GPROPERTY_SET_ERROR_INVALID_OBJECT,
	GPROPERTY_SET_ERROR_PROPERTY_WAS_NATIVE_CONST,
	GPROPERTY_SET_ERROR_INVALID_PROPERTY_TYPE


};

class GOBJECT_API GProperty
{
public:
	const char* get_name() const noexcept;
	
	GType get_type_info() const noexcept;

	GType get_parent_type_info() const noexcept;
	
	//X Getter Setter
	virtual std::expected<void, GPROPERTY_SET_ERROR> set(GVariant object, GVariant value) = 0;
	virtual std::expected<GVariant, GPROPERTY_GET_ERROR> get(GVariant object) = 0;

protected:
	GProperty(const char* name, const GTypeInfo* typeInfo, const GTypeInfo* parentInfo)
	{
		m_name = name;
		m_typeInfo = create_type(typeInfo);
		m_parentInfo = create_type(parentInfo);
	}
	std::string m_name;
	GType m_typeInfo;
	GType m_parentInfo;
};

#endif // GPROPERTY_H
