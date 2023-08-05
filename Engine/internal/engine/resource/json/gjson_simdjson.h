#ifndef GJSON_SIMDJSON_H
#define GJSON_SIMDJSON_H

#include "engine/resource/json/igjson.h"
#include <simdjson.h>

class GJson_SimdJson : public IGJson
{
public:
	GJson_SimdJson(const std::string& path);
	// Inherited via IGJson
	virtual RESOURCE_INIT_CODE init() override;
	virtual bool is_valid() override;

	virtual void destroy() override;

	virtual const char* get_file_path_c_str() override;

	virtual void iterate_in(std::function<void(std::string_view,const IGJsonValue*)> callback) override;

private:
	simdjson::fallback::ondemand::document m_doc;
	simdjson::ondemand::parser m_parser;
	bool m_is_valid = false;
	std::string m_filePath;
};

#endif // GJSON_SIMDJSON_H