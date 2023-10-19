#include "slotclass.h"

SlotClass::slot::signal_ref SlotClass::bind_slot(slot::func_type bind)
{
	return m_slot.connect(bind);
}

void SlotClass::trigger(int val)
{
	m_slot(val);
}
