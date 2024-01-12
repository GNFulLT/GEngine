#ifndef IGSCRIPT_INSTANCE_H
#define IGSCRIPT_INSTANCE_H

#include <cstdint>
#include "engine/GEngine_EXPORT.h"

class IGScript;
class GEntity;
class IGScriptObject;

class ENGINE_API IGScriptInstance
{
public:
	virtual ~IGScriptInstance() = default;

	virtual bool is_valid() const noexcept = 0;
	virtual void update(float dt) = 0;
	virtual void set_id(GEntity* entity) = 0;
	virtual IGScriptObject* get_script_object() const noexcept = 0;
protected:
	void set_script_id(IGScript* script,GEntity* entity);
private:
};

#endif // IGSCRIPT_INSTANCE_H