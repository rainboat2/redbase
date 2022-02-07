#include "IX_Internal.h"

IX_BBucketListWapper::IX_BBucketListWapper(int bucketItemNum, char* data, PageNum pageNum)
    : bucketItemNum_(bucketItemNum)
    , pageNum_(pageNum)
    , size_((int*)data)
    , bitmap_(data + sizeof(int))
{
    // bucketListSize := x
    // x := (PF_PAGE_SIZE - sizeof(int) - BITMAP_SIZE(x)) / recordSize
    int bucketSize = IX_BBucketWapper::memorySize(bucketItemNum);
    constexpr auto bitmapSize = BitMapWapper::memorySize;
    bucketListSize_ = (PF_PAGE_SIZE - sizeof(int)) / bucketSize;
    while (bucketListSize_ > 0 && (sizeof(int) + bitmapSize(bucketListSize_) + bucketListSize_ * bucketSize) > PF_PAGE_SIZE)
        bucketListSize_--;
    assert(bucketListSize_ > 0);

    bucketList_ = data + sizeof(int) + bitmapSize(bucketListSize_);
}

RID IX_BBucketListWapper::allocateBucket()
{
    assert(!isFull());
    BitMapWapper map(bitmap_, bucketListSize_);
    int i = map.findFirstZero();
    assert(i != -1);

    map.set(i, true);
    *size_ = *size_ + 1;

    auto bucket = get(i);
    IX_BBucketWapper::initBucket(bucket);
    return { pageNum_, i };
}

void IX_BBucketListWapper::initBucketList(PF_PageHandle& page, int bucketItemNum)
{
    assert(bucketItemNum > 0);
    char* data;
    page.GetData(data);
    memset(data, 0, PF_PAGE_SIZE);
}