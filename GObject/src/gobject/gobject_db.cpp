#include "gobject/gobject_db.h"
#include "gobject/gtype_info.h"
#include "gobject/gfunction_wrapper.h"
#include "internal/gobject_db_data.h"

GObjectDB::GObjectDB()
{
	//X TODO : PRIMITIVE TYPES
	m_data = new GObjectDBData();
}

GObjectDB::~GObjectDB()
{
	delete m_data;
}

GTypeInfo* GObjectDB::add_or_get_type_info(std::unique_ptr<GTypeInfo> typeInfo)
{
	if (const auto& oldTypeInfo = m_data->m_typeInfoTable.find(typeInfo->m_id); oldTypeInfo != m_data->m_typeInfoTable.end())
		return oldTypeInfo.operator->()->second.get();

	auto ptr = typeInfo.get();
	m_data->m_typeInfoTable.emplace(ptr->m_id, std::move(typeInfo));
	m_data->m_typeNameTable.emplace(ptr->m_name, ptr->m_id);
	return ptr;
}

GTypeInfo* GObjectDB::if_have_get_type(std::uint64_t id)
{
	if (const auto& oldTypeInfo = m_data->m_typeInfoTable.find(id); oldTypeInfo != m_data->m_typeInfoTable.end())
		return oldTypeInfo.operator->()->second.get();
	return nullptr;
}

GObjectDB& GObjectDB::get_object_db()
{
	static GObjectDB singleton;
	return singleton;
}

GTypeInfo* GObjectDB::if_have_get_type(const std::string& name)
{
	if (const auto& oldTypeInfo = m_data->m_typeNameTable.find(name); oldTypeInfo != m_data->m_typeNameTable.end())
	{
		std::uint64_t id = oldTypeInfo.operator->()->second;
		return m_data->m_typeInfoTable.find(id).operator->()->second.get();

	}
	return nullptr;
}

GFunctionWrapper* GObjectDB::define_function_for(GTypeInfo* type, std::unique_ptr<GFunctionWrapper> functionWrapper)
{
	if (type == nullptr || !type->m_is_valid || !functionWrapper)
	{
		return nullptr;
	}

	if (const auto& func = type->m_classInfo.m_functionMap.find(std::string(functionWrapper->get_name())); func != type->m_classInfo.m_functionMap.end())
	{
		return func.operator->()->second.get();
	}
	
	GFunctionWrapper* wrp = functionWrapper.get();
	type->m_classInfo.m_functionMap.emplace(wrp->get_name(), std::move(functionWrapper));
	return wrp;

}

GProperty* GObjectDB::define_property_for(GTypeInfo* type, std::unique_ptr<GProperty> property)
{
	if (type == nullptr || !type->m_is_valid || !property)
	{
		return nullptr;
	}
	if (const auto& prop = type->m_classInfo.m_propertyMap.find(std::string(property->get_name())); prop != type->m_classInfo.m_propertyMap.end())
	{
		return prop.operator->()->second.get();
	}
	auto wrp = std::shared_ptr<GProperty>(std::move(property));
	type->m_classInfo.m_propertyMap.emplace(wrp->get_name(), wrp);
	type->m_classInfo.m_properties.push_back(wrp);
	return wrp.get();
}

GTypeInfo* GObjectDB::get_type_info(uint64_t index)
{
	if (const auto& oldTypeInfo = m_data->m_typeInfoTable.find(index); oldTypeInfo != m_data->m_typeInfoTable.end())
	{
		return oldTypeInfo->second.get();
	}
	return nullptr;
}
