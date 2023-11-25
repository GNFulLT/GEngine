#ifndef GAPI_GENERAL_TYPES_H
#define GAPI_GENERAL_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct GNFPluginVersion
{
	uint32_t version_major = 0;
	uint32_t version_minor = 0;
} GNFPluginVersion;

typedef void* pGObject;


#endif // GAPI_GENERERAL_TYPES_H