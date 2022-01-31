#include "ix.h"

IX_IndexScan::IX_IndexScan()
    : isOpen_(false)
    , indexHandle_(nullptr)
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
    cmp_ = getComparator(fileHeader.attrType, fileHeader.attrLength);
    compOp_ = compOp;
    value_ = value;
    pinHint_ = pinHint;
    isOpen_ = true;
    hasNext_ = true;
    RETURN_RC_IF_NOT_SUCCESS(findFirstNode());
    return RC::SUCCESSS;
}

RC IX_IndexScan::CloseScan()
{
    if (!isOpen_)
        return RC::IX_INDEX_SCAN_CLOSED;

    if (curNode_.getPageNum() != NULL_PAGE_NUM)
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->unpinNode(curNode_));
    isOpen_ = false;
    hasNext_ = false;
    return RC::SUCCESSS;
}

RC IX_IndexScan::GetNextEntry(RID& rid)
{
    if (!isOpen_)
        return RC::IX_INDEX_SCAN_CLOSED;
    if (!hasNext_)
        return RC::IX_INDEX_SCAN_EOF;

    EXTRACT_SLOT_NUM(slot, cur_);
    rid = curNode_.getRid(slot);

    // move to next valid entry
    switch (compOp_) {
    case CompOp::EQ: {
        if (moveToNextEntry()) {
            EXTRACT_SLOT_NUM(nextSlot, cur_);
            hasNext_ = (cmp_(value_, curNode_.getAttr(nextSlot)) == 0);
        }
        break;
    }
    case CompOp::NE: {
        while (moveToNextEntry()) {
            EXTRACT_SLOT_NUM(nextSlot, cur_);
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
            EXTRACT_SLOT_NUM(nextSlot, cur_);
            hasNext_ = (cmp_(value_, curNode_.getAttr(nextSlot)) <= 0);
        }
        break;
    }
    case CompOp::LT: {
        if (moveToNextEntry()) {
            EXTRACT_SLOT_NUM(nextSlot, cur_);
            hasNext_ = (cmp_(value_, curNode_.getAttr(nextSlot)) < 0);
        }
        break;
    }
    default:
        assert(true);
    }
    return RC::SUCCESSS;
}

RC IX_IndexScan::findFirstNode()
{
    switch (compOp_) {
    case CompOp::EQ:
    case CompOp::GE:
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->GetLeafEntryAddrEqualTo(value_, cur_));
        break;
    case CompOp::GT:
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->GetLeafEntryAddrGreatThen(value_, cur_));
        break;
    case CompOp::NE:
    case CompOp::NO:
    case CompOp::LE:
    case CompOp::LT:
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->GetFirstLeafEntryAddr(cur_));
        break;
    default:
        assert(true);
    }
    EXTRACT_SLOT_NUM(slot, cur_);
    if (slot == -1)
        hasNext_ = false;
    else
        curNode_ = indexHandle_->readBNodeFrom(cur_);
    return RC::SUCCESSS;
}

bool IX_IndexScan::moveToNextEntry()
{
    EXTRACT_PAGE_SLOT_NUM(pageNum, slotNum, cur_);

    bool isMove = true;
    if (slotNum + 1 == curNode_.size()) {
        if (curNode_.lastRid() == NULL_RID) {
            isMove = isOpen_ = false;
        } else {
            RID next = curNode_.lastRid();
            indexHandle_->unpinNode(curNode_);
            curNode_ = indexHandle_->readBNodeFrom(next);
            EXTRACT_PAGE_NUM(nextPageNum, next);
            cur_ = RID(nextPageNum, 0);
        }
    } else {
        cur_ = RID(pageNum, slotNum + 1);
    }
    return isMove;
}
