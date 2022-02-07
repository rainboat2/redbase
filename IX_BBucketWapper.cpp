#include "IX_Internal.h"

IX_BBucketWapper::IX_BBucketWapper(int bucketItemNum, char* data)
    : itemNum_(bucketItemNum)
    , size_((int*)data)
    , rids_((RID*)(data + sizeof(int)))
{
}

void IX_BBucketWapper::addItem(RID rid)
{
    assert(!isFull());
    rids_[size()] = rid;
    *size_ += 1;
}

bool IX_BBucketWapper::deleteItem(RID rid)
{
    int index = -1;
    for (int i = 0; i < size(); i++) {
        if (get(i) == rid) {
            index = i;
            break;
        }
    }
    if (index == -1)
        return false;
    for (int i = index; i < size() - 1; i++) {
        rids_[i] = rids_[i + 1];
    }
    *size_ -= 1;
    return true;
}

void IX_BBucketWapper::initBucket(IX_BBucketWapper& bucket)
{
    *bucket.size_ = 0;
    bucket.setNext(NULL_RID);
}