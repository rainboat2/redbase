#include "ix.h"

IX_IndexHandle::IX_IndexHandle()
    : isOpen_(false)
    , isHeaderChange_(false)
{
}

IX_IndexHandle::~IX_IndexHandle()
{
}

RC IX_IndexHandle::DeleteEntry(void* pData, const RID& rid)
{
    DeleteEntry(root_, pData, rid, 0);
    return RC::SUCCESSS;
}

IX_BDeleteUpEntry IX_IndexHandle::DeleteEntry(IX_BNodeWapper& cur, void* pData, const RID& rid, int level)
{
    assert(level <= fileHeader_.height && level >= 0);
    IX_BDeleteUpEntry curEntry { false };
    if (level == fileHeader_.height - 1) {
        // leaf node
        cur.leafRemove(pData, rid);
        if (cur.size() < fileHeader_.order / 2)
            curEntry.needMerge = true;
    } else {
        // not leaf node
        int index = cur.upperBound(pData);
        IX_BNodeWapper next = readBNodeFrom(cur.getRid(index));
        auto up = DeleteEntry(next, pData, rid, level + 1);
        if (up.needMerge) {
            // TODO: redistribute entry
        }
        unpin(next);
    }
    return curEntry;
}

RC IX_IndexHandle::InsertEntry(void* pData, const RID& rid)
{
    IX_BInsertUpEntry up = InsertEntry(root_, pData, rid, 0);
    if (up.isSpilt)
        changeRoot(up);
    return RC::SUCCESSS;
}

IX_BInsertUpEntry IX_IndexHandle::InsertEntry(IX_BNodeWapper& cur, void* pData, const RID& rid, int level)
{
    assert(level <= fileHeader_.height && level >= 0);

    IX_BInsertUpEntry curEntry;
    if (level == fileHeader_.height - 1) {
        // leaf node
        if (cur.indexOf(pData) != -1) {
            insertIntoBucket(cur, pData, rid);
        } else {
            if (cur.isFull()) {
                IX_BNodeWapper newNode = createBNode();
                curEntry = cur.leafSpiltAndInsert(pData, rid, newNode);
                pf_fileHandle_.MarkDirty(newNode.getPageNum());
                pf_fileHandle_.UnpinPage(newNode.getPageNum());
            } else {
                cur.leafInsert(pData, rid);
            }
        }
    } else {
        // not leaf node
        int index = cur.upperBound(pData);
        IX_BNodeWapper next = readBNodeFrom(cur.getRid(index));
        auto upEntry = InsertEntry(next, pData, rid, level + 1);
        if (upEntry.isSpilt) {
            if (cur.isFull()) {
                IX_BNodeWapper newNode = createBNode();
                curEntry = cur.notLeafSpiltAndInsert(upEntry, newNode);
                pf_fileHandle_.MarkDirty(newNode.getPageNum());
                pf_fileHandle_.UnpinPage(newNode.getPageNum());
            } else {
                cur.notLeafInsert(upEntry);
            }
            pf_fileHandle_.MarkDirty(next.getPageNum());
        }
        unpin(next);
    }
    return curEntry;
}

/*
 * if pData corresponding to one RID only, the RID data store in node.
 * else we store the RIDs into bucket, and we keep bucketAddr(pageNum is negitive) in node.
 */
void IX_IndexHandle::insertIntoBucket(IX_BNodeWapper& leaf, void* pData, const RID& rid)
{
    int i = leaf.indexOf(pData);
    assert(i >= 0);
    RID tailAddr, curAddr = leaf.getRid(i);
    if (isBucketAddr(curAddr)) {
        tailAddr = findTail(curAddr);
        tailAddr = appendBucketIfFull(leaf, tailAddr, i);
    } else {
        tailAddr = findNewBucketFrom(leaf, i);
        leaf.setRid(i, tailAddr);
        markDirty(leaf);
    }

    auto bucketList = readBucketListFrom(tailAddr);
    EXTRACT_SLOT_NUM(slot, tailAddr);
    auto bucket = bucketList.get(slot);
    if (!isBucketAddr(curAddr)) {
        bucket.addItem(curAddr);
    }
    bucket.addItem(rid);
    markDirty(bucketList);
    unpin(bucketList);
}

RID IX_IndexHandle::appendBucketIfFull(IX_BNodeWapper& leaf, RID tailAddr, int pos)
{
    RID rs = tailAddr;
    auto bucketList = readBucketListFrom(tailAddr);
    EXTRACT_SLOT_NUM(slot, tailAddr);
    auto bucket = bucketList.get(slot);

    // append new bucket to the end if full.
    if (bucket.isFull()) {
        assert(bucket.next() == NULL_RID);
        RID newBucketAddr = findNewBucketFrom(leaf, pos);
        bucket.setNext(newBucketAddr);
        rs = newBucketAddr;
        markDirty(bucketList);
    }
    unpin(bucketList);
    return rs;
}

bool IX_IndexHandle::isBucketAddr(const RID& rid)
{
    assert(rid != NULL_RID);
    EXTRACT_PAGE_NUM(pageNum, rid);
    return pageNum < 0;
}

RID IX_IndexHandle::findTail(const RID& bucketAddr)
{
    RID cur = bucketAddr;
    bool isTail = false;
    while (!isTail) {
        auto bucketList = readBucketListFrom(cur);
        EXTRACT_SLOT_NUM(slot, cur);
        auto bucket = bucketList.get(slot);
        if (bucket.next() != NULL_RID) {
            cur = bucket.next();
        } else {
            isTail = true;
        }
        unpin(bucketList);
    }
    return cur;
}

RID IX_IndexHandle::findNewBucketFrom(IX_BNodeWapper& leaf, int i)
{
    RID rs = NULL_RID;
    // find left bucketList that contain empty bucket
    int p = i - 1;
    while (p >= 0 && !isBucketAddr(leaf.getRid(p))) {
        p--;
    }
    if (p >= 0) {
        auto bucketList = readBucketListFrom(leaf.getRid(p));
        if (!bucketList.isFull()) {
            rs = bucketList.allocateBucket();
            markDirty(bucketList);
        }
        unpin(bucketList);
    }

    // find right if find left failed
    if (rs == NULL_RID) {
        p = i + 1;
        while (p < leaf.size() && !isBucketAddr(leaf.getRid(p))) {
            p++;
        }
        if (p < leaf.size()) {
            auto bucketList = readBucketListFrom(leaf.getRid(p));
            if (!bucketList.isFull()) {
                rs = bucketList.allocateBucket();
                markDirty(bucketList);
            }
            unpin(bucketList);
        }
    }

    // allocate new bucketList if find both left and right failed
    if (rs == NULL_RID) {
        auto bucketList = createBucketList();
        rs = bucketList.allocateBucket();
        markDirty(bucketList);
        unpin(bucketList);
    }
    return rs;
}

RC IX_IndexHandle::GetLeafEntryAddrEqualTo(void* pData, RID& rid) const
{
    RC rc = getLeafBy(pData, rid, [](void* pData, IX_BNodeWapper& cur) {
        return cur.getRid(cur.upperBound(pData));
    });
    RETURN_RC_IF_NOT_SUCCESS(rc);

    // getLeafBy only find leaf node, we need to find slotNum that pData first appear
    auto leaf = readBNodeFrom(rid);
    SlotNum slot = leaf.indexOf(pData);
    rid = { leaf.getPageNum(), slot };
    unpin(leaf);
    return RC::SUCCESSS;
}

RC IX_IndexHandle::GetLeafEntryAddrGreatThen(void* pData, RID& rid) const
{
    RC rc = getLeafBy(pData, rid, [](void* pData, IX_BNodeWapper& cur) {
        return cur.getRid(cur.upperBound(pData));
    });
    RETURN_RC_IF_NOT_SUCCESS(rc);
    // getLeafBy only find leaf node, we need to find slotNum that it's data first large than pData
    auto leaf = readBNodeFrom(rid);
    SlotNum slot = leaf.upperBound(pData);
    if (slot == fileHeader_.order)
        slot = -1;
    rid = { leaf.getPageNum(), slot };
    return unpin(leaf);
}

RC IX_IndexHandle::GetFirstLeafEntryAddr(RID& rid) const
{
    char dummy[8];
    return getLeafBy(dummy, rid, [](void* pData, IX_BNodeWapper& cur) {
        return cur.getRid(0);
    });
    EXTRACT_PAGE_NUM(pageNum, rid);
    rid = { pageNum, 0 };
}

RC IX_IndexHandle::getLeafBy(void* pData, RID& rid, std::function<RID(void*, IX_BNodeWapper&)> getNext) const
{
    IX_BNodeWapper cur = root_;
    RID nextAddr;
    for (int i = 0; i < fileHeader_.height; i++) {
        nextAddr = getNext(pData, cur);
        if (i == fileHeader_.height - 1)
            rid = RID(cur.getPageNum(), 0);

        // if not root_, unpin page
        if (i != 0)
            RETURN_RC_IF_NOT_SUCCESS(unpin(cur));

        if (i != fileHeader_.height - 1)
            cur = readBNodeFrom(nextAddr);
    }
    return RC::SUCCESSS;
}

RC IX_IndexHandle::ForcePages()
{
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.ForcePages());
    return RC::SUCCESSS;
}

void IX_IndexHandle::changeRoot(IX_BInsertUpEntry& entry)
{
    IX_BNodeWapper newRoot = createBNode();
    newRoot.setAttr(0, entry.attr);
    newRoot.setRid(0, { root_.getPageNum(), 0 });
    newRoot.setRid(1, entry.right);
    pf_fileHandle_.UnpinPage(root_.getPageNum());
    fileHeader_.height += 1;
    fileHeader_.root = { newRoot.getPageNum(), 0 };
    isHeaderChange_ = true;
    root_ = newRoot;
}

IX_BNodeWapper IX_IndexHandle::readBNodeFrom(const RID& rid) const
{
    PageNum pageNum;
    rid.GetPageNum(pageNum);
    PF_PageHandle page;
    pf_fileHandle_.GetThisPage(pageNum, page);
    char* data;
    page.GetData(data);
    return IX_BNodeWapper(fileHeader_.attrLength, fileHeader_.attrType, data, rid);
}

IX_BBucketListWapper IX_IndexHandle::readBucketListFrom(const RID& rid) const
{
    assert(isBucketAddr(rid));
    EXTRACT_PAGE_NUM(pageNum, rid);
    PF_PageHandle page;
    pf_fileHandle_.GetThisPage(-pageNum, page);
    char* data;
    page.GetData(data);
    return IX_BBucketListWapper(fileHeader_.bucketItemNum, data, pageNum);
}

IX_BNodeWapper IX_IndexHandle::createBNode()
{
    PF_PageHandle page;
    RC rc = pf_fileHandle_.AllocatePage(page);
    assert(rc == RC::SUCCESSS);
    IX_BNodeWapper::initNode(page, fileHeader_.attrType, fileHeader_.attrLength);
    PageNum pageNum;
    char* data;
    page.GetData(data);
    page.GetPageNum(pageNum);

    fileHeader_.pageNums++;
    isHeaderChange_ = true;
    return IX_BNodeWapper(fileHeader_.attrLength, fileHeader_.attrType, data, { pageNum, 0 });
}

IX_BBucketListWapper IX_IndexHandle::createBucketList()
{
    PF_PageHandle page;
    RC rc = pf_fileHandle_.AllocatePage(page);
    assert(rc == RC::SUCCESSS);
    IX_BBucketListWapper::initBucketList(page, fileHeader_.bucketItemNum);
    char* data;
    page.GetData(data);
    PageNum pageNum;
    page.GetPageNum(pageNum);

    fileHeader_.pageNums++;
    isHeaderChange_ = true;
    return IX_BBucketListWapper(fileHeader_.bucketItemNum, data, pageNum);
}

RC IX_IndexHandle::forceHeader()
{
    if (!isHeaderChange_)
        return RC::SUCCESSS;
    PF_PageHandle page;
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(0, page));
    char* data;
    page.GetData(data);
    memcpy(data, &fileHeader_, sizeof(IX_BFileHeader));
    isHeaderChange_ = false;
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(0));
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(0));
    return RC::SUCCESSS;
}