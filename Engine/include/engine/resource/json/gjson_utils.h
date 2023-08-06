#ifndef GJSON_UTILS_H
#define GJSON_UTILS_H

#include "engine/GEngine_EXPORT.h"
#include "engine/resource/json/igjson.h"

#include <string>
#include <utility>
#include <tuple>
#include <expected>

enum class CONVERSION_ERROR
{
	CONVERSION_ERROR_UNEXPECTED = 0,
	CONVERSION_ERROR_NOT_ENOUGH_VALUE,
	CONVERSION_ERROR_IS_NOT_NUMERIC_HEXADECIMAL
};

class ENGINE_API GJsonUtils
{
public:
	static IGJson* create_default_json(const std::string& filePath);
	static std::expected<std::tuple<float, float, float, float>, CONVERSION_ERROR> string_to_rgba(std::string_view str);
};

#endif // GJSON_UTILS_H