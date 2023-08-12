#ifndef GINSTANCE_H
#define GINSTANCE_H



#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include "gobject/gtype.h"

class GOBJECT_API GInstance
{
public:
	bool operator=(const GInstance& other) _NO_EXCEPT_;

private:
	GType m_type;
	GType m_wrappedType;
	void* m_rawPointer;
	void* m_rawWrappedPointer;
};

#endif // GINSTANCE_H