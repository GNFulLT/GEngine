#ifndef IGJSON_VALUE_H
#define IGJSON_VALUE_H

enum JSON_VALUE
{
	JSON_VALUE_UNKNOWN = 0,
	JSON_VALUE_OBJECT = 1,
	JSON_VALUE_STRING,
	JSON_VALUE_UINT64,
	JSON_VALUE_UINT32,
	JSON_VALUE_INT64,
	JSON_VALUE_INT32,
	JSON_VALUE_DOUBLE,
	JSON_VALUE_CHAR,
	JSON_VALUE_BOOL,
	JSON_VALUE_ARRAY
};

#include <cstdint>
#include <string_view>

class IGJsonValue
{
public:
	~IGJsonValue() = default;	

	virtual JSON_VALUE get_value_type() = 0;

	virtual uint64_t try_to_get_as_uint64() = 0;

	virtual std::string_view try_to_get_as_string() = 0;
	
	virtual int64_t try_to_get_as_int64() = 0;
	
	virtual bool try_to_get_as_bool() = 0;
	
	virtual double try_to_get_as_double() = 0;

private:
};

#endif // IGJSON_VALUE_H