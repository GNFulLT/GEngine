#ifndef IMANAGER_TABLE_H
#define IMANAGER_TABLE_H

#include "GEngine_EXPORT.h"
#include <cstdint>
#include "public/core/templates/shared_ptr.h"


class GEngine;

#define GET_MANAGER(MANAGER_TYPE,ENUM_NAME) ((GSharedPtr<MANAGER_TYPE>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENUM_NAME))->get()
//X TODO : USE BETTER ENUM
enum ENGINE_MANAGER : uint8_t 
{
	ENGINE_MANAGER_WINDOW = 0,
	ENGINE_MANAGER_GRAPHIC_DEVICE,
	ENGINE_MANAGER_PIPELINE_OBJECT,
	ENGINE_MANAGER_LOGGER,
	ENGINE_MANAGER_RESOURCE,
	ENGINE_MANAGER_JOB,
	ENGINE_MANAGER_SHADER,
	ENGINE_MANAGER_CAMERA,
	ENGINE_MANAGER_SCENE,
	ENGINE_MANAGER_SCRIPT,
	ENGINE_MANAGER_PROJECT
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