#ifndef ISERIALIZABLE_H
#define ISERIALIZABLE_H

#include "gobject/gtype.h"
#include "gobject/gvariant.h"

class ISerializable
{
public:
	virtual ~ISerializable() = default;

	virtual GType get_type() const = 0;

	virtual GVariant as_variant() = 0;
private:
};

#endif // ISERIALIZABLE_H