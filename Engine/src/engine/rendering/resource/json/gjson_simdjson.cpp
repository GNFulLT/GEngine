#include "internal/engine/resource/json/gjson_simdjson.h"
#include "internal/engine/resource/json/gjson_value_simdjson.h"

RESOURCE_INIT_CODE GJson_SimdJson::init()
{
	//X TODO : ERROR CODES HERE

	auto res = simdjson::padded_string::load(m_filePath);

	auto err = res.error();
	if (err)
		return RESOURCE_INIT_CODE_UNKNOWN_EX;

	auto rr  = m_parser.iterate(res.value_unsafe());
	
	m_doc = std::move(rr.value_unsafe());

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

void GJson_SimdJson::iterate_in(std::function<void(std::string_view,IGJsonValue*)> callback)
{
	auto objRes = m_doc.get_object();
	auto err = objRes.error();
	if (err)
		return;
	for (const simdjson::simdjson_result<simdjson::ondemand::field>& field : objRes.value_unsafe())
	{
		if (field.error())
			return;

		simdjson::fallback::ondemand::field f = field.value_unsafe();
		simdjson::fallback::ondemand::value val = f.value();
		
		GJsonValue_SimdJson valueAs(&val);
		callback(std::string_view(f.key().raw()), &valueAs);
		
	}


}
