#ifndef GOBJECT_UTILS
#define GOBJECT_UTILS



#include "gobject/gobject.h"
#include "gobject/gtype_info.h"
#include "gobject/gfunction_wrapper.h"
#include "gobject/gproperty_wrapper.h"

namespace GTypeUtils
{

	template<typename T>
	inline GType get_type_from_instance(T* ptr)
	{
		return create_type(GObjectDB::get_object_db().add_or_get_type_info(create_type_info_for<T>()));
	}

	template<typename T>
	inline GType get_type_from_type()
	{
		return create_type(GObjectDB::get_object_db().add_or_get_type_info(create_type_info_for<T>()));
	}

	template<typename C,typename R,typename... Args>
	inline GFunctionWrapper* register_member_function(GTypeInfo* inf,std::string_view name, R(C::*fn)(Args...))
	{
		GFunctionWrapper* wrp = new GMemberFunctionWrapper(name,fn);
		return GObjectDB::get_object_db().define_function_for(inf,std::unique_ptr<GFunctionWrapper>(wrp));
	}

	template<typename C, typename T>
	inline GProperty* register_property(GTypeInfo* inf, std::string_view name, T C::* dataMemPtr)
	{
		GProperty* wrp = new GPropertyRaw<C,T>(dataMemPtr,name.data());
		return GObjectDB::get_object_db().define_property_for(inf, std::unique_ptr<GProperty>(wrp));
	}
}



#define GOBJECT_ENABLE(type) static void registeration_func_##type(); \
struct gobject_definitior##type {	\
public: gobject_definitior##type() { registeration_func_##type(); } \
} static gobject_definitior##type; \
static void registeration_func_##type() { GTypeInfo* inf = GObjectDB::get_object_db().add_or_get_type_info(create_type_info_for<type>()); 

#define GOBJECT_DEFINE_MEMBER_METHOD(name,func) GTypeUtils::register_member_function(inf,name,func);

#define GOBJECT_DEFINE_PROPERTY(name,prop) GTypeUtils::register_property(inf,name,prop);

#endif // GOBJECT_UTILS