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

IX_BNodeWapper::IX_BNodeWapper(int attrLength, AttrType attrType, char* nodeData, RID addr)
    : attrLength_(attrLength)
    , attrType_(attrType)
    , order_(countOrder(attrLength))
    , addr_(addr)
    , raw_data_(nodeData)
    , size_((int*)raw_data_)
    , rids_((RID*)((char*)raw_data_ + sizeof(int) + attrLength * order_))
    , attrs_((char*)raw_data_ + sizeof(int))
{
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
        while (rs + step < size() && cmp_(pData, getAttr(rs + step)) >= 0) {
            tmp = rs + step;
            step = step << 1;
        }
        if (tmp == rs)
            break;
        else
            rs = tmp;
    }
    // find the first element greater than pdata
    if (cmp_(pData, getAttr(rs)) > 0) rs++;
    return rs;
}

int IX_BNodeWapper::insert(void* attr, RID rid)
{
    assert(*size_ <= order_ && *size_ >= 0);
    if (*size_ == order_)
        return -1;
    // insert sort
    setAttr(*size_, attr);
    setRid(*size_, rid);
    int i = *size_;
    while (i > 0 && cmp_(getAttr(i - 1), getAttr(i)) > 0)
        i--;
    return i;
}

IX_BInsertUpEntry IX_BNodeWapper::insertAndSpilt(void* attr, RID rid, IX_BNodeWapper &newNode, bool isCopyUp){
    assert(isFull());
}

void IX_BNodeWapper::initNode(PF_PageHandle& page)
{
    char* data;
    page.GetData(data);
    *((int*)data) = 0;
}

void IX_BNodeWapper::swap(int i, int j)
{
    char buffer[MAX_STRING_LEN];
    memcpy(buffer, getAttr(i), attrLength_);
    memcpy(getAttr(i), getAttr(j), attrLength_);
    memcpy(getAttr(j), buffer, attrLength_);

    RID tmp = getRid(i);
    setRid(i, getRid(j));
    setRid(j, tmp);
}

/*
* suitation 1:
* order_ = 2 * n + 1,
* n + 1 attr to first node, and n attr to second node
* 
* sutiation 2:
* order_ = 2 * n + 2
* n + 1 attr to first node, and n + 1 attr to second node
* last entry will be insert to second node for both suitations
*/
void IX_BNodeWapper::spiltInto(IX_BNodeWapper &other, IX_BInsertUpEntry &up){

}