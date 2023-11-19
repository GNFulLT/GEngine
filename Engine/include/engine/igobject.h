#ifndef IGOBJECT_H
#define IGOBJECT_H

#include "engine/iserializable.h"
#include "gobject/gtype.h"
#include "gobject/gtype_utils.h"
#include "gobject/gobject_defs.h"

#define GOBJECT_DEF(CLASS_NAME,PARENT_CLASS) 	virtual GType get_type() const override { 	return create_type(add_or_get_type_info<CLASS_NAME>());  } \
			virtual GVariant as_variant() override{ return  GVariant(GVariantRef(*((CLASS_NAME*)this))); }

class IGObject : public ISerializable
{
public:
	virtual GType get_type() const override
	{
		return GType();
	}

	virtual GVariant as_variant() override
	{
		return GVariant();
	}
private:
};

#endif // IGOBJECT_H