#ifndef GSCRIPT_INSTANCE_1_0_H
#define GSCRIPT_INSTANCE_1_0_H

#include "engine/plugin/gplugin.h"
#include "internal/engine/plugin/gscript_object_1_0.h"
#include "engine/plugin/igscript_instance.h"

class IGScript;

class GScriptInstance_1_0 : public IGScriptInstance
{
public:
	GScriptInstance_1_0(GScriptObject_1_0* owner,GNFScriptClassConstructor scriptCtor, GNFScriptClassDestructor scriptDtor);
	~GScriptInstance_1_0();
	bool is_valid() const noexcept override;

	void update(float dt) override;

	void destroy_internal();
private:
	bool isValid = false;
	GScriptObject_1_0* m_owner = nullptr;
	IGScript* m_script = nullptr;
	GNFScriptClassDestructor m_scriptDtor;

	// Inherited via IGScriptInstance
	virtual void set_id(GEntity* entity) override;

	// Inherited via IGScriptInstance
	virtual IGScriptObject* get_script_object() const noexcept override;
};

#endif // GSCRIPT_INSTANCE_1_0_H