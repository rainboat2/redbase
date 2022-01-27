#include "ix.h"

IX_IndexScan::IX_IndexScan()
    : isOpen_(false)
    , indexHandle_(nullptr)
{
}

IX_IndexScan::~IX_IndexScan() { }

RC IX_IndexScan(const IX_IndexHandle& indexHandle,
    CompOp compOp,
    void* value,
    ClientHint pinHint)
{
    return RC::SUCCESSS;
}