#include "ix.h"

IX_IndexHandle::IX_IndexHandle()
    : isOpen_(false)
{
}

RC IX_IndexHandle::InsertEntry(void* pData, const RID &rid){
    return RC::SUCCESSS;
}