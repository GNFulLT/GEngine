#include "gobject/gproperty.h"
#include "gobject/gproperty_wrapper.h"

GProperty::GProperty()
{
	m_wrapper = nullptr;
}

GProperty::GProperty(GPropertyWrapper* wrapper)
{
	m_wrapper = wrapper;
}

std::expected<void, GPROPERTY_SET_ERROR> GProperty::set(GVariant object, GVariant value)
{
	if (m_wrapper == nullptr)
		return std::unexpected(GPROPERTY_SET_ERROR_INVALID_PROPERTY);
	return m_wrapper->set(object, value);
}

std::expected<GVariant, GPROPERTY_GET_ERROR> GProperty::get(GVariant object)
{
	if (m_wrapper == nullptr)
		return std::unexpected(GPROPERTY_GET_ERROR_INVALID_PROPERTY);
	return m_wrapper->get(object);
}

const char* GProperty::get_name() const noexcept
{
	return m_wrapper->m_name.c_str();
}

GType GProperty::get_type_info() const noexcept
{
	return m_wrapper->m_typeInfo;
}

GType GProperty::get_parent_type_info() const noexcept
{
	return m_wrapper->m_parentInfo;
}
