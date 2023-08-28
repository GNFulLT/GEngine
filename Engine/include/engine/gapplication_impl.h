#ifndef GAPPLICATION_IMPL_H
#define GAPPLICATION_IMPL_H

#include "GEngine_EXPORT.h"

class GEngine;
class IInjectManagerHelper;

class ENGINE_API GApplicationImpl
{
public:
	virtual ~GApplicationImpl() = default;

	virtual bool before_update() = 0;

	virtual void update() = 0;

	virtual void after_update() = 0;

	virtual bool before_render() = 0;

	virtual void render() = 0;

	virtual void after_render() = 0;

	virtual bool init(GEngine* engine) = 0;

	virtual void destroy() = 0;

	virtual void inject_managers(IInjectManagerHelper* helper) {}
private:
};


#endif // GAPPLICATION_IMPL_H