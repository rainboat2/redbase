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
    , rids_((RID*)(raw_data_ + sizeof(int) + attrLength * order_))
    , attrs_(raw_data_ + sizeof(int))
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
    if (cmp_(pData, getAttr(rs)) > 0)
        rs++;
    return rs;
}

int IX_BNodeWapper::leafInsert(void* attr, RID left)
{
    assert(*size_ <= order_ && *size_ >= 0);
    if (isFull())
        return -1;

    int i = indexOf(attr);
    insertInto(i, left);
    insertInto(i, (char*)attr);
    *size_ += 1;
    return i;
}

int IX_BNodeWapper::notLeafInsert(const IX_BInsertUpEntry& up)
{
    assert(*size_ <= order_ && *size_ >= 0);
    if (isFull())
        return -1;

    int i = indexOf(up.attr);
    insertInto(i + 1, up.right);
    insertInto(i, (char*)up.attr);
    *size_ += 1;
    return i;
}

IX_BInsertUpEntry IX_BNodeWapper::leafSpiltAndInsert(void* attr, RID rid, IX_BNodeWapper& newNode)
{
    assert(isFull());
    int i = indexOf(attr);

    char last_attr[MAX_STRING_LEN];
    RID last_rid;
    if (i == *size_) {
        memcpy(last_attr, attr, attrLength_);
        last_rid = rid;
    } else {
        memcpy(last_attr, getAttr(*size_ - 1), attrLength_);
        last_rid = getRid(*size_ - 1);
        *size_ -= 1;
        leafInsert(attr, rid);
    }
    IX_BInsertUpEntry cur;
    spiltInto(newNode, cur, true);
    newNode.leafInsert(last_attr, last_rid);
    return cur;
}

IX_BInsertUpEntry IX_BNodeWapper::notLeafSpiltAndInsert(const IX_BInsertUpEntry &up, IX_BNodeWapper &newNode){
    assert(isFull());
    IX_BInsertUpEntry last;
    int i = indexOf(up.attr);
    if (i == order_){
        last = up;
    }else{
        memcpy(last.attr, getAttr(*size_ - 1), attrLength_);
        last.right = getRid(*size_ - 1);
        *size_ -= 1;
        notLeafInsert(last);
    }
    IX_BInsertUpEntry cur;
    spiltInto(newNode, cur, false);
    newNode.notLeafInsert(last);
    return cur;
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
* order_ = 2 * n + 1, (assume n = 1)
* 2 attr to first node, and 1 attr to second node
* mid = 2 = (order_ + 1) / 2
*
* sutiation 2:
* order_ = 2 * n + 2, (assume n = 1)
* 2 attr to first node, and 2 attr to second node 
* mid = 2 =  (order_ + 1) / 2
* note: mid indicate the first entry of second node)
*/
void IX_BNodeWapper::spiltInto(IX_BNodeWapper& second, IX_BInsertUpEntry& up, bool isLeaf)
{
    assert(order_ == *size_);
    int mid = (order_ + 1) / 2;

    // spilt node
    *size_ = mid;
    if (isLeaf) {
        *second.size_ = order_ - mid;
        // the last rid point to next sibling, so there is no need to set it here
        memcpy(second.rids_, rids_ + mid * sizeof(RID), (order_ - mid) * sizeof(RID));
        memcpy(second.attrs_, attrs_ + mid * attrLength_, (order_ - mid) * attrLength_);
    } else {
        *second.size_ = order_ - mid - 1;
        // the rid belong to mid entry should be kept in first node.
        memcpy(second.rids_, rids_ + (mid + 1) * sizeof(RID), (order_ - mid) * sizeof(RID));
        memcpy(second.attrs_, attrs_ + (mid + 1) * sizeof(RID), (order_ - mid - 1) * attrLength_);
    }

    memcpy(up.attr, getAttr(mid), attrLength_);
    up.left = { getPageNum(), 0 };
    up.right = { second.getPageNum(), 0 };
    up.isSpilt = true;

    if (isLeaf) {
        // set next sibling
        second.rids_[order_] = { getPageNum(), -1 };
        rids_[order_] = { second.getPageNum(), -1 };
    }
}

void IX_BNodeWapper::insertInto(int i, RID rid)
{
    assert(*size_ < order_ && i <= *size_);
    for (int j = *size_; j > i; j++) {
        setRid(j, getRid(j - 1));
    }
    setRid(i, rid);
}

void IX_BNodeWapper::insertInto(int i, char* pData)
{
    assert(*size_ < order_ && i <= *size_);
    for (int j = *size_; j > i; j++) {
        setAttr(j, getAttr(j - 1));
    }
    setAttr(i, pData);
}