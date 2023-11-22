#include "engine/io/json_utils.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <fstream>
#include <rapidjson/ostreamwrapper.h>
#include "internal/engine/resource/json/gjson_simdjson.h"

bool GJsonUtils::serialize_igobject(std::filesystem::path path, IGObject* object)
{
	auto objectType = object->get_type();
	auto objectVariant = object->as_variant();

	if (!objectType.is_valid() || !objectVariant.is_valid())
		return false;

	auto properties = objectType.get_properties();
	rapidjson::Document document;
	auto& allocator = document.GetAllocator();

	std::ofstream stream(path, std::ofstream::trunc);
	rapidjson::OStreamWrapper ostream(stream);
	rapidjson::Writer<rapidjson::OStreamWrapper> ostreamWriter(ostream);


	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> pwriter(s);

	pwriter.StartObject();

	for (auto& property : properties)
	{
		auto propName = property.get_name();
		auto propValVariantRes = property.get(objectVariant);
		if (!propValVariantRes.has_value())
		{
			//X TODO : LOG
			continue;
		}
		auto propValVariant = propValVariantRes.value();
		auto valAsRaw = propValVariant.get_raw();
		if (propValVariant.is(GTypeUtils::add_or_get_type<std::string>()))
		{
			auto val = (std::string*)valAsRaw;
			pwriter.Key(propName);
			pwriter.String(val->c_str());
			//document[propName] = (*val);
		
		}
		else if (propValVariant.is(GTypeUtils::add_or_get_type<int>()))
		{
			auto val = (int*)valAsRaw;
			pwriter.Key(propName);
			pwriter.Int(*val);
		}
		else if (propValVariant.is(GTypeUtils::add_or_get_type<std::uint32_t>()))
		{
			auto val = (uint32_t*)valAsRaw;
			pwriter.Uint(*val);

		}
	}

	pwriter.EndObject();

	document.Parse(s.GetString());

	document.Accept(ostreamWriter);

	ostreamWriter.Flush();
	stream.close();
	
	return true;
}

void GJsonUtils::set_serialized_values_from_json(std::filesystem::path path, IGObject* object)
{
	auto objectType = object->get_type();
	auto objectVariant = object->as_variant();

	if (!objectType.is_valid() || !objectVariant.is_valid())
		return;

	GJson_SimdJson simd(path.string());


	auto isJson = simd.iterate_in([&](std::string_view key,IGJsonValue* val) {
			auto keyS = std::string(key.data(), key.size());
			GProperty prop = objectType.get_property_by_name(keyS.c_str());
			if (!prop.is_valid())
			{
				return;
			}
			auto propType = prop.get_type_info();
			auto json_val = val->get_value_type();
			//X TODO : FILTER
			switch (json_val)
			{
			case JSON_VALUE_UNKNOWN:
				break;
			case JSON_VALUE_OBJECT:
				break;
			case JSON_VALUE_STRING:
				if (propType.equals(GTypeUtils::add_or_get_type<std::string>()))
				{
					std::string valS = std::string(val->try_to_get_as_string());
					auto valSRef = GVariantRef(valS);
					auto res = prop.set(objectVariant, GVariant(valSRef));
					if (!res.has_value())
					{
						//X TODO : LOG
					}
				}
				break;
			case JSON_VALUE_UINT64:
				break;
			case JSON_VALUE_INT64:
				break;
			case JSON_VALUE_DOUBLE:
				break;
			case JSON_VALUE_CHAR:
				break;
			case JSON_VALUE_BOOL:
				break;
			case JSON_VALUE_ARRAY:
				break;
			default:
				break;
			}
		});
}
