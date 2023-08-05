#ifndef MANAGER_TABLE_H
#define MANAGER_TABLE_H

#include "engine/imanager_table.h"
#include "public/core/templates/unordered_dense.h"
class ManagerTable : public IManagerTable
{
public:
	virtual void* get_engine_manager_managed(ENGINE_MANAGER manager) override;

	virtual void* get_engine_manager_raw(ENGINE_MANAGER manager) override;

	void set_manager(ENGINE_MANAGER manager, void* pManager);

	//X This methos just delete managers from RAM. Dont forget to call destroy methods of managers before use it !
	void delete_managers();
private:
	ankerl::unordered_dense::map<int, void*> m_manager_map;
	
};

#endif // MANAGER_TABLE_H