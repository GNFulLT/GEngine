#ifndef IMANAGER_TABLE_H
#define IMANAGER_TABLE_H

#include "GEngine_EXPORT.h"
#include <cstdint>

//X TODO : USE BETTER ENUM
enum ENGINE_MANAGER : uint8_t 
{
	ENGINE_MANAGER_WINDOW = 0,
	ENGINE_MANAGER_GRAPHIC_DEVICE,
	ENGINE_MANAGER_LOGGER,
	ENGINE_MANAGER_RESOURCE,
	ENGINE_MANAGER_JOB,
	ENGINE_MANAGER_SHADER,
	ENGINE_MANAGER_CAMERA
};

class ENGINE_API IManagerTable
{
public:
	virtual ~IManagerTable() = default;


	virtual void* get_engine_manager_managed(ENGINE_MANAGER manager) = 0;
	virtual void* get_engine_manager_raw(ENGINE_MANAGER manager) = 0;
private:
};

#endif // MANAGER_TABLE_H