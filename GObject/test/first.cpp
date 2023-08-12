#include "first.h"

#include "gobject/gobject_utils.h"

GOBJECT_ENABLE(First)
	GOBJECT_DEFINE_MEMBER_METHOD("b",&First::b)
}


int First::b(int a)
{
	return 1;
}
