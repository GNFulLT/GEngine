#include "C:\Users\lenovo\Desktop\GEngine\GObject\src\gobject\gobject_utils.h"
#include "C:\Users\lenovo\Desktop\GEngine\GObject\test\first.h"



GOBJECT_ENABLE(First)
	GOBJECT_DEFINE_PROPERTY("number",&First::number)
}
GOBJECT_ENABLE(Second)
	GOBJECT_DEFINE_PROPERTY("number",&Second::number)
	GOBJECT_DEFINE_PROPERTY("s",&Second::s)
	GOBJECT_DEFINE_PROPERTY("a",&Second::a)
}
