#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "engine/GEngine_EXPORT.h"
#include "engine/igobject.h"
#include <filesystem>

class ENGINE_API GJsonUtils
{
public:
	static bool serialize_igobject(std::filesystem::path path,IGObject* object);
	
	static void set_serialized_values_from_json(std::filesystem::path path, IGObject* object);
private:
};

#endif // JSON_UTILS_H