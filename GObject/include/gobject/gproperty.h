#ifndef GPROPERTY_H
#define GPROPERTY_H

#include <string>

#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include "gobject/gproperty_errors.h"
#include <expected>

class GType;
class GVariant;
class GPropertyWrapper;

class GOBJECT_API GProperty
{
public:
	GProperty();
	GProperty(GPropertyWrapper* wrapper);
	//X Getter Setter
	std::expected<void, GPROPERTY_SET_ERROR> set(GVariant object, GVariant value);
	std::expected<GVariant, GPROPERTY_GET_ERROR> get(GVariant object);

	const char* get_name() const noexcept;

	GType get_type_info() const noexcept;

	GType get_parent_type_info() const noexcept;

	bool is_valid() const noexcept;
private:
	GPropertyWrapper* m_wrapper;
};
#endif // GPROPERTY_H
