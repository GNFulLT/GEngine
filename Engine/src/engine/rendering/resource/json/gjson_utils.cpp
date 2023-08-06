#include "engine/resource/json/gjson_utils.h"

#include "internal/engine/resource/json/gjson_simdjson.h"
#include "public/core/string/gchar.h"

std::expected<std::tuple<float, float, float, float>, CONVERSION_ERROR> hex_string_to_rgba(std::string_view str);

IGJson* GJsonUtils::create_default_json(const std::string& filePath)
{
	//X TODO : GDNEWDA 
	return new GJson_SimdJson(filePath);
}

std::expected<std::tuple<float, float, float, float>, CONVERSION_ERROR> GJsonUtils::string_to_rgba(std::string_view str)
{
	if (str.size() == 0)
		return std::unexpected(CONVERSION_ERROR::CONVERSION_ERROR_UNEXPECTED);

	if (str[0] == '#')
	{
		return hex_string_to_rgba(str);
	}

	return std::unexpected(CONVERSION_ERROR::CONVERSION_ERROR_UNEXPECTED);
}

std::expected<std::tuple<float, float, float, float>, CONVERSION_ERROR> hex_string_to_rgba(std::string_view str)
{
	std::string strAs;
	strAs.resize(8);
	if (str.size() == 4)
	{
		
		strAs[0] = std::toupper(str[1]);
		strAs[1] = std::toupper(str[1]);
		strAs[2] = std::toupper(str[2]);
		strAs[3] = std::toupper(str[2]);
		strAs[4] = std::toupper(str[3]);
		strAs[5] = std::toupper(str[3]);
		strAs[6] = 'F';
		strAs[7] = 'F';
	}
	else if(str.size() == 7)
	{
		strAs[0] = std::toupper(str[1]);
		strAs[1] = std::toupper(str[2]);
		strAs[2] = std::toupper(str[3]);
		strAs[3] = std::toupper(str[4]);
		strAs[4] = std::toupper(str[5]);
		strAs[5] = std::toupper(str[6]);
		strAs[6] = 'F';
		strAs[7] = 'F';
	}
	else if (str.size() == 9)
	{
		strAs[0] = std::toupper(str[1]);
		strAs[1] = std::toupper(str[2]);
		strAs[2] = std::toupper(str[3]);
		strAs[3] = std::toupper(str[4]);
		strAs[4] = std::toupper(str[5]);
		strAs[5] = std::toupper(str[6]);
		strAs[6] = std::toupper(str[7]);
		strAs[7] = std::toupper(str[8]);
	}
	else
	{
		return std::unexpected(CONVERSION_ERROR::CONVERSION_ERROR_NOT_ENOUGH_VALUE);
	}
	for (int i = 0; i < 8; i++)
	{
		if (!GCharA::is_numeric_hexadecimal(strAs[i]))
			return std::unexpected(CONVERSION_ERROR::CONVERSION_ERROR_IS_NOT_NUMERIC_HEXADECIMAL);
	}

	int r, g, b,a;
	auto res = sscanf(strAs.c_str(), "%02x%02x%02x%02x", &r, &g, &b,&a);
	return std::tuple<float, float, float, float>((float)r/255.f,(float)g/255.f,(float)b/255.f,(float)a/255.f);
}