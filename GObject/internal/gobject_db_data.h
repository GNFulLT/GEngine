#ifndef GOBJECT_DB_DATA_H
#define GOBJECT_DB_DATA_H

#include "public/core/templates/unordered_dense.h"
#include "gobject/gtype_info.h"
#include <cstdint>

struct GObjectDBData
{
	ankerl::unordered_dense::segmented_map<std::uint64_t, std::unique_ptr<GTypeInfo>> m_typeInfoTable;
	ankerl::unordered_dense::segmented_map<std::string, std::uint64_t> m_typeNameTable;
};

#endif // GOBJECT_DB_DATA_H