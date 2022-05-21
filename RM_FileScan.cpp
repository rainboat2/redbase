#include "rm.h"
#include <cassert>

RM_FileScan::RM_FileScan()
    : fileHandle_(nullptr)
    , isOpen_(false)
    , cur_({ 1, -1 })
{
}

RC RM_FileScan::OpenScan(
    const RM_FileHandle& fileHandle,
    AttrType attrType,
    int attrLength,
    int attrOffset,
    CompOp compOp,
    void* value,
    ClientHint pinHint)
{
    if (isOpen_)
        return RC::RM_FILE_SCAN_OPENED;
    assert((attrType == AttrType::RD_INT && attrLength == sizeof(int))
        || (attrType == AttrType::RD_FLOAT && attrLength == sizeof(float))
        || (attrType == AttrType::RD_STRING && attrLength > 0 && attrLength < MAX_STRING_LEN));

    fileHandle_ = const_cast<RM_FileHandle*>(&fileHandle);
    attrType_ = attrType;
    attrLength_ = attrLength;
    attrOffset_ = attrOffset;
    compOp_ = compOp;
    value_ = value;
    pinHint_ = pinHint;
    cur_ = RID { 1, -1 };
    isOpen_ = true;
    return RC::SUCCESS;
}

RC RM_FileScan::GetNextRec(RM_Record& rec)
{
    if (!isOpen_) return RC::RM_FILE_SCAN_CLOSED;
    RM_Record tmp;
    while (true) {
        RETURN_RC_IF_NOT_SUCCESS(fileHandle_->GetNextRec(cur_, tmp));
        tmp.GetRid(cur_);
        if (isMatch(tmp)) {
            rec = std::move(tmp);
            break;
        }
    }
    return RC::SUCCESS;
}

RC RM_FileScan::CloseScan()
{
    if (!isOpen_)
        return RC::RM_FILE_SCAN_CLOSED;
    isOpen_ = false;
    return RC::SUCCESS;
}

bool RM_FileScan::isMatch(RM_Record& rec)
{
    if (value_ == nullptr || compOp_ == CompOp::NO)
        return true;
    char* buf_;
    rec.GetData(buf_);
    char* val = buf_ + attrOffset_;
    auto cmp = getComparator(attrType_, attrLength_);

    switch (compOp_) {
    case CompOp::EQ:
        return cmp(val, value_) == 0;
    case CompOp::NE:
        return cmp(val, value_) != 0;
    case CompOp::GE:
        return cmp(val, value_) >= 0;
    case CompOp::GT:
        return cmp(val, value_) > 0;
    case CompOp::LE:
        return cmp(val, value_) <= 0;
    case CompOp::LT:
        return cmp(val, value_) < 0;
    case CompOp::NO:
        return true;
    default:
        assert(false);
    }
}