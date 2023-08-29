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

    //X TODO : OPTIMIZE

    if (endsWith(str, ".txt"))
    {
        return FILE_TYPE_TXT;
    }
    else
    {
        return FILE_TYPE_TXT;
    }
}
