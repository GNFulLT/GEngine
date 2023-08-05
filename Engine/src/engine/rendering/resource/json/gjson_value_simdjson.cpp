#include "internal/engine/resource/json/gjson_value_simdjson.h"

#include <stdexcept>

GJsonValue_SimdJson::GJsonValue_SimdJson(simdjson::fallback::ondemand::value* val)
{
	m_valRef = val;
}

JSON_VALUE GJsonValue_SimdJson::get_value_type()
{
	auto res = m_valRef->type();

	if (res.error())
		return JSON_VALUE_UNKNOWN;

	auto valType = res.value_unsafe();
	
	using namespace simdjson::fallback;
	switch (valType)
	{
		case ondemand::json_type::array:
			return JSON_VALUE_ARRAY;
		case ondemand::json_type::boolean:
			return JSON_VALUE_BOOL;
		case ondemand::json_type::object:
			return JSON_VALUE_OBJECT;
		case ondemand::json_type::string:
			return JSON_VALUE_STRING;
		case ondemand::json_type::number:
			auto res2 = m_valRef->get_number_type();
			if (res2.error())
				return JSON_VALUE_UNKNOWN;
			switch (res2.value_unsafe())
			{
			case number_type::unsigned_integer:
				return JSON_VALUE_UINT64;
			case number_type::signed_integer:
				return JSON_VALUE_INT64;
			case number_type::floating_point_number:
				return JSON_VALUE_DOUBLE;
			default:
				return JSON_VALUE_UNKNOWN;
			}
		default:
			return JSON_VALUE_UNKNOWN;
	}
}

uint64_t GJsonValue_SimdJson::try_to_get_as_uint64()
{
	auto val = m_valRef->get_uint64();

	//X TODO : Custom Exception

	if (val.error())
		throw std::runtime_error("Couldn't convert to value");

	return val.value_unsafe();
}

std::string_view GJsonValue_SimdJson::try_to_get_as_string()
{
	auto val = m_valRef->get_string();

	//X TODO : Custom Exception

	if (val.error())
		throw std::runtime_error("Couldn't convert to value");

	return val.value_unsafe();
}

int64_t GJsonValue_SimdJson::try_to_get_as_int64()
{
	auto val = m_valRef->get_int64();

	//X TODO : Custom Exception

	if (val.error())
		throw std::runtime_error("Couldn't convert to value");

	return val.value_unsafe();
}

bool GJsonValue_SimdJson::try_to_get_as_bool()
{
	auto val = m_valRef->get_bool();

	//X TODO : Custom Exception

	if (val.error())
		throw std::runtime_error("Couldn't convert to value");

	return val.value_unsafe();
}

double GJsonValue_SimdJson::try_to_get_as_double()
{
	auto val = m_valRef->get_double();

	//X TODO : Custom Exception

	if (val.error())
		throw std::runtime_error("Couldn't convert to value");

	return val.value_unsafe();
}

