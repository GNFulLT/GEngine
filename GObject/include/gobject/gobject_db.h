#ifndef GOBJECT_DB_H
#define GOBJECT_DB_H

#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"

#include <memory>
#include <cstdint>
#include <string>
#include <functional>
#include <type_traits>

struct GTypeInfo;
class GFunctionWrapper;
struct GObjectDBData;

class GOBJECT_API GObjectDB
{
public:
	GObjectDB();
	~GObjectDB();

	GTypeInfo* add_or_get_type_info(std::unique_ptr<GTypeInfo> typeInfo);


	GTypeInfo* if_have_get_type(std::uint64_t id);


	static GObjectDB& get_object_db();

	GTypeInfo* if_have_get_type(const std::string& name);

	template<typename Func>
	void register_method(std::string_view name, Func f)
	{	
		
		static_assert(std::is_function<Func>::value, "Given property is not a function");
	}
	
	GFunctionWrapper* define_function_for(GTypeInfo* type,std::unique_ptr<GFunctionWrapper> functionWrapper);

	GTypeInfo* get_type_info(uint64_t index);
	
private:
	GObjectDBData* m_data;

};




#endif // GOBJECT_DB_H
