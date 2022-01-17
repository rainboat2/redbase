#include "rm.h"

RM_FileScan::RM_FileScan()
    : fileHandle_(nullptr)
    , isOpen_(false)
    , cur({ 1, -1 })
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
        return RC::RM_FILE_SCAN_ALREAD_OPEN;
    fileHandle_ = const_cast<RM_FileHandle*>(&fileHandle);

    return RC::SUCCESSS;
}
