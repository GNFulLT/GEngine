#include "internal/utils.h"
#include <cstring>

int endsWith(const char* s, const char* part)
{
	return (strstr(s, part) - s) == (strlen(s) - strlen(part));
}

FILE_TYPE get_file_type_from_name(const char* str)
{
	int i = 0;
    int exclamationCheck = -1;
    while (str[i] != '\0') {
        if (str[i] == '.') {
            exclamationCheck = i;
            break;
        }
        i++;
    }
    if (exclamationCheck == -1)
        return FILE_TYPE_FOLDER;

    //X TODO : OPTIMIZE WITH UNORDERED SEGMANTED MAP 

    if (endsWith(str, ".txt"))
    {
        return FILE_TYPE_TXT;
    }
    else if(endsWith(str, ".hlsl"))
    {
        return FILE_TYPE_HLSL;
    }
    else if (endsWith(str, ".glsl"))
    {
        return FILE_TYPE_GLSL;
    }
    else if(endsWith(str, ".c"))
    {
        return FILE_TYPE_C_SRC;
    }
    else if (endsWith(str, ".h"))
    {
        return FILE_TYPE_C_HEADER;
    }
    else if (endsWith(str, ".cpp"))
    {
        return FILE_TYPE_CPP_SRC;
    }
    else if (endsWith(str, ".json"))
    {
        return FILE_TYPE_JSON;
    }
    else {
        return FILE_TYPE_UNKNOWN;
    }
}
