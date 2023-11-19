#ifndef SERIALIZABLE_COMPONENT_H
#define SERIALIZABLE_COMPONENT_H

#include "gobject/gtype.h"
#include "gobject/gtype_utils.h"
#include "engine/iserializable.h"

template<typename T>
class SerializableComponent : ISerializable
{
public:
	virtual GType get_type() const override;

	virtual GVariant as_variant() override;
private:
};

template<typename T>
inline GType SerializableComponent<T>::get_type() const
{
	return create_type(add_or_get_type_info<T>());
}

template<typename T>
inline GVariant SerializableComponent<T>::as_variant()
{
	return GVariant(GVariantRef(*((T*)this)));
}
#endif // SERIALIZABLE_COMPONENT_H

