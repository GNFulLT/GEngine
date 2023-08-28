#ifndef GINJECTION_MANAGER_H
#define GINJECTION_MANAGER_H

#include "engine/manager/iinject_manager_helper.h"

#include <unordered_map>

class GInjectManagerHelper : public IInjectManagerHelper
{
public:
	// Inherited via IInjectManagerHelper
	virtual void* get_manager_spec(ENGINE_MANAGER manager) override;


	void add_manager_spec(ENGINE_MANAGER, void*);
private:
	std::unordered_map<ENGINE_MANAGER, void*> m_map;

};

#endif // GINJECTION_MANAGER_H