#ifndef IINJECT_MANAGER_HELPER_H
#define IINJECT_MANAGER_HELPER_H

#include "engine/GEngine_EXPORT.h"

#include "engine/imanager_table.h"

class ENGINE_API IInjectManagerHelper
{
public:
	virtual ~IInjectManagerHelper() = default;

	virtual void* get_manager_spec(ENGINE_MANAGER manager) = 0;

	virtual void delete_and_swap(ENGINE_MANAGER manager, void* mngPtr) = 0;

	virtual void* swap_and_get_managed(ENGINE_MANAGER mng, void* ptr) = 0;

private:
};

#endif // IINJECT_MANAGER_HELPER_H