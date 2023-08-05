#include "engine/resource/json/gjson_utils.h"

#include "internal/engine/resource/json/gjson_simdjson.h"

IGJson* GJsonUtils::create_default_json(const std::string& filePath)
{
	//X TODO : GDNEWDA 
	return new GJson_SimdJson(filePath);
}
