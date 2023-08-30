#ifndef GINJECTION_MANAGER_H
#define GINJECTION_MANAGER_H

#include "engine/manager/iinject_manager_helper.h"
#include "internal/engine/manager_table.h"
#include <unordered_map>


class GInjectManagerHelper : public IInjectManagerHelper
{
public:
	GInjectManagerHelper(ManagerTable* table);
	// Inherited via IInjectManagerHelper
	virtual void* get_manager_spec(ENGINE_MANAGER manager) override;

	virtual void delete_and_swap(ENGINE_MANAGER manager, void* mngPtr) override;

	void add_manager_spec(ENGINE_MANAGER, void*);
private:
	std::unordered_map<ENGINE_MANAGER, void*> m_map;
	ManagerTable* m_table;

};

#endif // GINJECTION_MANAGER_H