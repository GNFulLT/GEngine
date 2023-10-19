#ifndef SLOTCLASS_H
#define SLOTCLASS_H

#include "public/core/string/gstring.h"
#include "public/core/templates/shared_ptr.h"
#include <gtest/gtest.h>
#include "public/core/templates/signal/gslot.h"

class SlotClass
{
public:
	typedef GSlot<void(int)> slot;

	slot::signal_ref bind_slot(slot::func_type bind);
	void trigger(int val);
private:
	slot m_slot;
};

#endif // SLOTCLASS_H