#ifndef GTYPE_UTILS_H
#define GTYPE_UTILS_H

#include "gobject/gobject_db.h"
#include "gobject/gtype_info.h"

namespace GTypeUtils
{
	template<typename T>
	inline GTypeInfo* add_or_get_type_info()
	{
		return GObjectDB::get_object_db().add_or_get_type_info(create_type_info_for<T>());
	}
	template<typename T>
	inline GType add_or_get_type()
	{
		return create_type(GObjectDB::get_object_db().add_or_get_type_info(create_type_info_for<T>()));
	}
	
}
template<typename T>
inline GTypeInfo* add_or_get_type_info()
{
	return GObjectDB::get_object_db().add_or_get_type_info(create_type_info_for<T>());
}
#endif