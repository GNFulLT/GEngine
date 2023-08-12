#ifndef GTYPE_H
#define GTYPE_H

#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include <string_view>
#include "gobject/gfunction.h"

struct GTypeInfo;

class GOBJECT_API GType
{
public:
	GType();

	GType(const GTypeInfo* inf);
	std::string_view get_name() const _NO_EXCEPT_;

	bool is_valid() const _NO_EXCEPT_;

	bool equals(const GType& other) const _NO_EXCEPT_;

	GFunction get_function_by_name(std::string_view name) _NO_EXCEPT_;

	bool is_class() const _NO_EXCEPT_;
private:
	const GTypeInfo* m_info;
};


GOBJECT_API GType create_type(const GTypeInfo* inf);

#endif // GTYPE_H