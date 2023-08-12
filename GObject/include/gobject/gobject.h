#ifndef GOBJECT_H
#define GOBJECT_H

#include "public/typedefs.h"
#include "gobject/gtype.h"
#include "gobject/gobject_db.h"
#include "gobject/gfunction.h"

namespace GTypeUtils
{
	inline GOBJECT_API GType get_type_from_name(const char* name)
	{
		return create_type(GObjectDB::get_object_db().if_have_get_type(name));
	}


	template<typename T>
	inline GOBJECT_API GType get_type_of()
	{
		return create_type(GObjectDB::get_object_db().get_type_info(typeid(T).hash_code()));
	}

}



#endif // GOBJECT_H