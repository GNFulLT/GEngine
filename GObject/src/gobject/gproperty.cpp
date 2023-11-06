#include "gobject/gproperty.h"

const char* GProperty::get_name() const noexcept
{
	return m_name.c_str();
}

GType GProperty::get_type_info() const noexcept
{
	return m_typeInfo;
}

GType GProperty::get_parent_type_info() const noexcept
{
	return m_parentInfo;
}
