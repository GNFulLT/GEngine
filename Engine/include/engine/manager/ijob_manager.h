#ifndef IJOB_MANAGER_H
#define IJOB_MANAGER_H

#include "engine/GEngine_EXPORT.h"

#include <functional>
#include "engine/time_unit.h"

class ENGINE_API IJobManager
{
public:
	typedef void(*fire_and_forget_fn)(void*);
	virtual ~IJobManager() = default;

	
	virtual void fire_and_forget(fire_and_forget_fn,void* arg) = 0;

	virtual void fire_and_forget(fire_and_forget_fn,void* arg,int timeout,TIME_UNIT unit) = 0;
private:

};

#endif // IJOB_MANAGER_H