#include "ix.h"

IX_IndexScan::IX_IndexScan()
    : indexHandle_(nullptr)
    , isOpen_(false)
{
}

IX_IndexScan::~IX_IndexScan() { }

RC IX_IndexScan::OpenScan(const IX_IndexHandle& indexHandle,
    CompOp compOp,
    void* value,
    ClientHint pinHint)
{
    if (isOpen_)
        return RC::IX_INDEX_SCAN_OPENED;
    const IX_BFileHeader& fileHeader = indexHandle.fileHeader_;
    indexHandle_ = const_cast<IX_IndexHandle*>(&indexHandle);
    cmp_ = getComparator(fileHeader.attrType, fileHeader.attrLength);
    compOp_ = compOp;
    value_ = value;
    pinHint_ = pinHint;
    isOpen_ = true;
    hasNextBucket_ = true;
    RETURN_RC_IF_NOT_SUCCESS(findFirstNode());
    return RC::SUCCESSS;
}

RC IX_IndexScan::CloseScan()
{
    if (!isOpen_)
        return RC::IX_INDEX_SCAN_CLOSED;

    if (curNode_.getPageNum() != NULL_PAGE_NUM)
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->unpin(curNode_));
    isOpen_ = false;
    hasNextBucket_ = false;
    return RC::SUCCESSS;
}

RC IX_IndexScan::GetNextEntry(RID& rid)
{
    if (!isOpen_)
        return RC::IX_INDEX_SCAN_CLOSED;
    if (!hasNextBucket_)
        return RC::IX_INDEX_SCAN_EOF;

    if (indexHandle_->isBucketAddr(curRid())) {
        assert(bucketIt_.hasNext());
        rid = bucketIt_.next();
        if (!bucketIt_.hasNext())
            moveToNextValidEntry();
    } else {
        rid = curRid();
        moveToNextValidEntry();
    }
    return RC::SUCCESSS;
}

RC IX_IndexScan::findFirstNode()
{
    switch (compOp_) {
    case CompOp::EQ:
    case CompOp::GE:
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->GetLeafEntryAddrEqualTo(value_, curPos_));
        break;
    case CompOp::GT:
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->GetLeafEntryAddrGreatThen(value_, curPos_));
        break;
    case CompOp::NE:
    case CompOp::NO:
    case CompOp::LE:
    case CompOp::LT:
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->GetFirstLeafEntryAddr(curPos_));
        break;
    default:
        assert(true);
    }
    EXTRACT_SLOT_NUM(slot, curPos_);
    if (slot == -1) {
        hasNextBucket_ = false;
    } else {
        curNode_ = indexHandle_->readBNodeFrom(curPos_);
        if (indexHandle_->isBucketAddr(curRid()))
            bucketIt_ = indexHandle_->getBucketIterator(curRid());
    }
    return RC::SUCCESSS;
}

void IX_IndexScan::moveToNextValidEntry()
{
    switch (compOp_) {
    case CompOp::EQ: {
        if (moveToNextEntry()) {
            EXTRACT_SLOT_NUM(nextSlot, curPos_);
            hasNextBucket_ = (cmp_(value_, curNode_.getAttr(nextSlot)) == 0);
        }
        break;
    }
    case CompOp::NE: {
        while (moveToNextEntry()) {
            EXTRACT_SLOT_NUM(nextSlot, curPos_);
            if (cmp_(value_, curNode_.getAttr(nextSlot)) != 0)
                break;
        }
        break;
    }
    case CompOp::NO:
    case CompOp::GE:
    case CompOp::GT: {
        moveToNextEntry();
        break;
    }
    case CompOp::LE: {
        if (moveToNextEntry()) {
            EXTRACT_SLOT_NUM(nextSlot, curPos_);
            hasNextBucket_ = (cmp_(curNode_.getAttr(nextSlot), value_) <= 0);
        }
        break;
    }
    case CompOp::LT: {
        if (moveToNextEntry()) {
            EXTRACT_SLOT_NUM(nextSlot, curPos_);
            hasNextBucket_ = (cmp_(curNode_.getAttr(nextSlot), value_) < 0);
        }
        break;
    }
    default:
        assert(true);
    }
    if (hasNextBucket_ && indexHandle_->isBucketAddr(curRid()))
        bucketIt_ = indexHandle_->getBucketIterator(curRid());
}

bool IX_IndexScan::moveToNextEntry()
{
    EXTRACT_PAGE_SLOT_NUM(pageNum, slotNum, curPos_);

    bool isMove = true;
    if (slotNum + 1 == curNode_.size()) {
        if (curNode_.lastRid() == NULL_RID) {
            isMove = hasNextBucket_ = false;
        } else {
            RID next = curNode_.lastRid();
            indexHandle_->unpin(curNode_);
            curNode_ = indexHandle_->readBNodeFrom(next);
            EXTRACT_PAGE_NUM(nextPageNum, next);
            curPos_ = RID(nextPageNum, 0);
        }
    } else {
        curPos_ = RID(pageNum, slotNum + 1);
    }
    return isMove;
}
