#include "IX_Internal.h"

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

IX_BNodeWapper::IX_BNodeWapper(int attrLength, AttrType attrType, char* data)
    : attrLength_(attrLength)
    , attrType_(attrType)
    , raw_data_(data)
    , order_(countOrder(attrLength))
    , size_((int*)data)
{
    attrs_ = (char*)raw_data_ + sizeof(int);
    rids_ = (RID*)((char*)attrs_ + attrLength * order_);

    if (attrType_ == AttrType::RD_INT) {
        cmp_ = int_cmp;
    } else if (attrType_ == AttrType::RD_FLOAT) {
        cmp_ = float_cmp;
    } else {
        cmp_ = [attrLength](const void* d1, const void* d2) {
            return strncmp((char*)d1, (char*)d2, attrLength);
        };
    }
}

int IX_BNodeWapper::indexOf(const void* pData) const
{
    int rs = 0;
    while (true) {
        int step = 1, tmp = rs;
        while (rs + step < size() && cmp_(pData, getData(rs + step)) >= 0) {
                tmp = rs + step;
            step = step << 1;
        }
        if (tmp == rs)
            break;
        else
            rs = tmp;
    }
    return rs;
}

void IX_BNodeWapper::initNode(PF_PageHandle &page){
    char *data;
    page.GetData(data);
    *((int*)data) = 0;
}