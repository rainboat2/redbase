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
    int i = 0;
    while (i < itemNum_ && get(i) == NULL_RID)
        i++;
    assert(i != itemNum_);
    rids_[i] = rid;
    *size_ += 1;
}

void IX_BBucketWapper::initBucket(IX_BBucketWapper& bucket)
{
    *bucket.size_ = 0;
    for (int i = 0; i <= bucket.itemNum_; i++) {
        bucket.rids_[i] = NULL_RID;
    }
}