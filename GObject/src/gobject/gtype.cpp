#include "gobject/gtype.h"
#include "gobject/gtype_info.h"

GOBJECT_API GType create_type(const GTypeInfo* inf)
{
	if (inf == nullptr)
		return GType();
	return GType(inf);
}

GType::GType()
{
	m_info = nullptr;
}

GType::GType(const GTypeInfo* inf)
{
	m_info = inf;
}
const std::vector<std::shared_ptr<GProperty>>* GType::get_properties() _NO_EXCEPT_
{
	return &m_info->m_classInfo.m_properties;
}

std::string_view GType::get_name() const _NO_EXCEPT_
{
	return m_info->m_name;
}

bool GType::equals(const GType& other) const _NO_EXCEPT_
{
	if (m_info == nullptr || other.m_info == nullptr)
		return false;

	return m_info->m_id == other.m_info->m_id;
}

bool GType::is_valid() const _NO_EXCEPT_
{
	return m_info != nullptr;
}


bool GType::is_class() const _NO_EXCEPT_
{
	return true;
}

GFunction GType::get_function_by_name(std::string_view name) _NO_EXCEPT_
{
	if (!is_valid() || !is_class())
		return GFunction();

	if (auto t = m_info->m_classInfo.m_functionMap.find(std::string(name)); t != m_info->m_classInfo.m_functionMap.end())
	{
		return GFunction(t->second.get());
	}
	return GFunction();
}