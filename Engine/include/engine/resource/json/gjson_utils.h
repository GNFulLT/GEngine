#ifndef GJSON_UTILS_H
#define GJSON_UTILS_H

#include "engine/GEngine_EXPORT.h"
#include "engine/resource/json/igjson.h"

#include <string>

class ENGINE_API GJsonUtils
{
public:
	static IGJson* create_default_json(const std::string& filePath);
};

#endif // GJSON_UTILS_H