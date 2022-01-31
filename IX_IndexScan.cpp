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
    findFirstNode();
    return RC::SUCCESSS;
}

RC IX_IndexScan::CloseScan()
{
    if (!isOpen_)
        return RC::IX_INDEX_SCAN_CLOSED;

    indexHandle_->unpinNode(curNode_);
    isOpen_ = false;
    return RC::SUCCESSS;
}

RC IX_IndexScan::GetNextEntry(RID& rid)
{
    if (!isOpen_)
        return RC::IX_INDEX_SCAN_CLOSED;

    switch (compOp_) {
    case CompOp::EQ:
        EXTRACT_SLOT_NUM(slot, cur_);
        rid = curNode_.getRid(slot);
        if (moveToNext()) {
            EXTRACT_SLOT_NUM(nextSlot, cur_);
            isOpen_ = (cmp_(value_, curNode_.getAttr(nextSlot)) == 0);
        }
        break;
    default:
        assert(true);
    }
    return RC::SUCCESSS;
}

RC IX_IndexScan::findFirstNode()
{
    switch (compOp_) {
    case CompOp::EQ:
        RETURN_RC_IF_NOT_SUCCESS(indexHandle_->GetLeafEntryAddrEqualTo(value_, cur_));
        EXTRACT_SLOT_NUM(slot, cur_);
        if (slot == -1)
            CloseScan();
    default:
        assert(true);
    }
    return RC::SUCCESSS;
}

bool IX_IndexScan::moveToNext()
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
