#ifndef GJSON_H
#define GJSON_H

#include "engine/GEngine_EXPORT.h"
#include <string_view>
#include "engine/resource/resource_init_code.h"
#include "engine/resource/json/igjson_value.h"

#include <functional>
#include <utility>

class ENGINE_API IGJson
{
public:
	virtual ~IGJson() = default;

	virtual void iterate_in(std::function<void(std::string_view,const IGJsonValue*)> callback) = 0;

	virtual RESOURCE_INIT_CODE init() = 0;

	virtual bool is_valid() = 0;

	virtual void destroy() = 0;

	virtual const char* get_file_path_c_str() = 0;

private:
};

#endif // GJSON_H