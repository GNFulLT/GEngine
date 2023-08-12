#include "gobject/gvariant.h"
#include "gobject/gtype_info.h"

GVariant::GVariant()
{
	m_rawData = nullptr;
}

GVariant::GVariant(GVariantRef& ref)
{
	m_rawData = ref.m_instance;
	m_type = ref.m_type;
}

bool GVariant::is_valid() const _NO_EXCEPT_
{
	return m_rawData != nullptr && m_type.is_valid();
}

bool GVariant::is(const GType& type) const
{
	return type.equals(m_type);
}

const GType& GVariant::get_type()
{
	return m_type;
}

void* GVariant::get_raw()
{
	return m_rawData;
}