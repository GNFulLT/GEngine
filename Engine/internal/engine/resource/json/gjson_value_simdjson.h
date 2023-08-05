#ifndef GJSON_VALUE_SIMDJSON_H
#define GJSON_VALUE_SIMDJSON_H

#include "engine/resource/json/igjson_value.h"
#include <simdjson.h>
class GJsonValue_SimdJson : public IGJsonValue
{
public:
	GJsonValue_SimdJson(simdjson::fallback::ondemand::value* val);

	// Inherited via IGJsonValue
	virtual JSON_VALUE get_value_type();

	// Inherited via IGJsonValue
	virtual uint64_t try_to_get_as_uint64();

	virtual std::string_view try_to_get_as_string();

	virtual int64_t try_to_get_as_int64();

	virtual bool try_to_get_as_bool();

	virtual double try_to_get_as_double();
private:
	simdjson::fallback::ondemand::value* m_valRef;

};

#endif // GSON_VALUE_SIMDJSON_H