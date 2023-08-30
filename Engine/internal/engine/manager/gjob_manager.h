#ifndef GJOB_MANAGER_H
#define GJOB_MANAGER_H

#include "engine/manager/ijob_manager.h"

class GJobManager : public IJobManager
{
public:

	// Inherited via IJobManager
	virtual void fire_and_forget(fire_and_forget_fn,void* arg) override;
	virtual void fire_and_forget(fire_and_forget_fn, void* arg, int timeout, TIME_UNIT unit) override;
private:

};


#endif // GJOB_MANAGER_H