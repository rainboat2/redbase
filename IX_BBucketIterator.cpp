#include "ix.h"
#include <cassert>

IX_BBucketIterator::IX_BBucketIterator()
    : indexHandle_(nullptr)
{
}

IX_BBucketIterator::IX_BBucketIterator(const IX_IndexHandle* indexHandle, RID bucketAddr)
    : indexHandle_(indexHandle)
    , pos_(0)
{
    bucketList_ = indexHandle_->readBucketListFrom(bucketAddr);
    EXTRACT_SLOT_NUM(slot, bucketAddr);
    bucket_ = bucketList_.get(slot);
    assert(bucket_.get(pos_) != NULL_RID);
}

IX_BBucketIterator::~IX_BBucketIterator()
{
    if (indexHandle_ != nullptr)
        indexHandle_->unpin(bucketList_);
}

bool IX_BBucketIterator::hasNext() const
{
    return pos_ != bucket_.size() || bucket_.next() != NULL_RID;
}

RID IX_BBucketIterator::next()
{
    assert(hasNext());
    RID rs = bucket_.get(pos_);
    // move to next item
    pos_++;
    if (pos_ == bucket_.size() && bucket_.next() != NULL_RID) {
        RID next = bucket_.next();
        indexHandle_->unpin(bucketList_);
        bucketList_ = indexHandle_->readBucketListFrom(next);
        EXTRACT_SLOT_NUM(slot, next);
        bucket_ = bucketList_.get(slot);
        pos_ = 0;
        // bucket should not empty
        assert(bucket_.size() != 0);
    }

    return rs;
}