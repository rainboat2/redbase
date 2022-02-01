#include "IX_Internal.h"

IX_BNodeWapper::IX_BNodeWapper()
    : addr_(NULL_RID)
{
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
    cmp_ = getComparator(attrType_, attrLength_);
}

int IX_BNodeWapper::upperBound(const void* pData) const
{
    int rs = 0;
    while (true) {
        int step = 1, tmp = rs;
        while (rs + step < size() && cmp_(getAttr(rs + step), pData) <= 0) {
            tmp = rs + step;
            step = step << 1;
        }
        if (tmp == rs)
            break;

        else
            rs = tmp;
    }
    if (rs < size() && cmp_(getAttr(rs), pData) <= 0)
        rs ++;
    return rs;
}

int IX_BNodeWapper::lowerBound(const void* pData) const
{
    int rs = -1;
    while (true) {
        int step = 1, tmp = rs;
        while (rs + step < size() && cmp_(getAttr(rs + step), pData) < 0) {
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

int IX_BNodeWapper::indexOf(const void* pData) const
{
    int rs = lowerBound(pData);
    if (rs + 1 < size() && cmp_(getAttr(rs + 1), pData) == 0)
        rs++;
    else
        rs = -1;
    return rs;
}

int IX_BNodeWapper::leafInsert(void* attr, RID left)
{
    assert(*size_ <= order_ && *size_ >= 0);
    if (isFull())
        return -1;

    int i = upperBound(attr);
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

    int i = upperBound(up.attr);
    insertInto(i + 1, up.right);
    insertInto(i, (char*)up.attr);
    *size_ += 1;
    return i;
}

IX_BInsertUpEntry IX_BNodeWapper::leafSpiltAndInsert(void* attr, RID rid, IX_BNodeWapper& newNode)
{
    assert(isFull());
    int i = upperBound(attr);

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

IX_BInsertUpEntry IX_BNodeWapper::notLeafSpiltAndInsert(const IX_BInsertUpEntry& up, IX_BNodeWapper& newNode)
{
    assert(isFull());
    IX_BInsertUpEntry last;
    int i = upperBound(up.attr);
    if (i == order_) {
        last = up;
    } else {
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
        memcpy(second.rids_, rids_ + mid, (order_ - mid) * sizeof(RID));
        memcpy(second.attrs_, attrs_ + mid * attrLength_, (order_ - mid) * attrLength_);
    } else {
        *second.size_ = order_ - mid - 1;
        // the rid belong to mid entry should be kept in first node.
        memcpy(second.rids_, rids_ + (mid + 1), (order_ - mid) * sizeof(RID));
        memcpy(second.attrs_, attrs_ + (mid + 1) * attrLength_, (order_ - mid - 1) * attrLength_);
    }

    memcpy(up.attr, getAttr(mid), attrLength_);

    up.right = { second.getPageNum(), 0 };
    up.isSpilt = true;

    if (isLeaf) {
        // set next sibling
        second.rids_[order_] = lastRid();
        rids_[order_] = { second.getPageNum(), NULL_SLOT_NUM };
    }
}

void IX_BNodeWapper::insertInto(int i, RID rid)
{
    for (int j = *size_; j > i; j--) {
        setRid(j, getRid(j - 1));
    }
    setRid(i, rid);
}

void IX_BNodeWapper::insertInto(int i, char* pData)
{
    for (int j = *size_; j > i; j--) {
        setAttr(j, getAttr(j - 1));
    }
    setAttr(i, pData);
}

void IX_BNodeWapper::initNode(PF_PageHandle& page, AttrType attrType_, int attrLength_)
{
    char* data;
    page.GetData(data);
    IX_BNodeWapper node(attrLength_, attrType_, data, NULL_RID);
    *node.size_ = 0;
    node.setRid(node.order_, NULL_RID);
}