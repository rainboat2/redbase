#include "redbase.h"

#include <string.h>

int int_cmp(const void* d1, const void* d2)
{
    int v1 = *((int*)d1), v2 = *((int*)d2);
    if (v1 == v2)
        return 0;
    else if (v1 > v2)
        return 1;
    else
        return -1;
}

int float_cmp(const void* d1, const void* d2)
{
    float f1 = *((float*)d1), f2 = *((int*)d2);
    if (f1 == f2)
        return 0;
    else if (f1 > f2)
        return 1;
    else
        return -1;
}

std::function<int(const void* d1, const void* d2)> getComparator(AttrType attrType, int attrLength)
{
    if (attrType == AttrType::RD_INT) {
        return int_cmp;
    } else if (attrType == AttrType::RD_FLOAT) {
        return float_cmp;
    } else {
        return [attrLength](const void* d1, const void* d2) {
            return strncmp((char*)d1, (char*)d2, attrLength);
        };
    }
}