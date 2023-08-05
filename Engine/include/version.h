#ifndef VERSION_H
#define VERSION_H

#include <string_view>
#include "public/typedefs.h"

#define GNF_VERSION_MAJOR 1

#define GNF_VERSION_MINOR 0

#define GNF_VERSION "" GNF_VERSION_MAJOR "." GNF_VERSION_MINOR

#define GNF_VERSION_AS_HEX 0x1000 * GNF_VERSION_MAJOR + 0x100 * GNF_VERSION_MINOR 

#define GNF_APP GEngine

#define GNF_APP_NAME_FULL _STR_XDEF(GNF_APP) "v" GNF_VERSION


struct VersionInfo
{
	uint32_t major;
	uint32_t minor;
	uint32_t hex;
};

#endif // VERSION_H