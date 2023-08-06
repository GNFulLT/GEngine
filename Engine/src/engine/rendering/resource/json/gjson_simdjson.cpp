#include "internal/engine/resource/json/gjson_simdjson.h"
#include "internal/engine/resource/json/gjson_value_simdjson.h"

RESOURCE_INIT_CODE GJson_SimdJson::init()
{
	//X TODO : ERROR CODES HERE

	auto res = simdjson::padded_string::load(m_filePath);

	auto err = res.error();
	if (err)
		return RESOURCE_INIT_CODE_UNKNOWN_EX;

	m_is_valid = true;

	return RESOURCE_INIT_CODE_OK;
}

GJson_SimdJson::GJson_SimdJson(const std::string& path)
{
	m_filePath = path;
}

bool GJson_SimdJson::is_valid()
{
	return m_is_valid;
}

void GJson_SimdJson::destroy()
{
}

const char* GJson_SimdJson::get_file_path_c_str()
{
	return m_filePath.c_str();
}

bool GJson_SimdJson::iterate_in(std::function<void(std::string_view,IGJsonValue*)> callback)
{
	auto res = simdjson::padded_string::load(m_filePath);

	auto err = res.error();

	if (err)
		return false;

	auto rr = m_parser.iterate(res.value_unsafe());

	auto doc = std::move(rr.value_unsafe());
	auto objRes = doc.get_object();
	err = objRes.error();
	if (err)
		return false;
	for (const simdjson::simdjson_result<simdjson::ondemand::field>& field : objRes.value_unsafe())
	{
		if (field.error())
			return false;

		simdjson::fallback::ondemand::field f = field.value_unsafe();
		simdjson::fallback::ondemand::value val = f.value();
		
		GJsonValue_SimdJson valueAs(&val);
		auto keyRes = f.unescaped_key(false);
		if (keyRes.error())
			return false;
		callback(keyRes.value_unsafe(), &valueAs);
		
	}

	return true;
}
