#include "RM_Internal.h"
#include "redbase.h"
#include "rm.h"

#define RETURN_IF_INVALID_RID(rid)                                          \
    {                                                                       \
        PageNum pageNum;                                                    \
        rid.GetPageNum(pageNum);                                            \
        /* first page is used as header, so pageNum should greater than 0*/ \
        if (pageNum >= fileHeader_.pageNums || pageNum <= 0)                \
            return RC::RM_INVALID_RID;                                      \
        SlotNum slotNum;                                                    \
        rid.GetSlotNum(slotNum);                                            \
        if (slotNum >= fileHeader_.recordNumsOfEachPage || slotNum < 0)     \
            return RC::RM_INVALID_RID;                                      \
    }

#define SLOT_OFFSET(slotNum) RM_PageHeader::size(fileHeader_.recordSize) + slotNum * fileHeader_.recordSize

RM_FileHandle::RM_FileHandle()
    : isOpen(false)
{
}

RM_FileHandle::~RM_FileHandle()
{
    ForcePages(ALL_PAGES);
}

RC RM_FileHandle::GetRec(const RID& rid, RM_Record& rec) const
{
    if (!isOpen)
        return RC::RM_FILE_NOT_OPEN;
    RETURN_IF_INVALID_RID(rid);

    PF_PageHandle page;
    PageNum pageNum;
    rid.GetPageNum(pageNum);
    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(pageNum, page));

    char* data;
    page.GetData(data);
    SlotNum slotNum;
    rid.GetSlotNum(slotNum);
    rec.setData(data + SLOT_OFFSET(slotNum), fileHeader_.recordSize);
    rec.rid_ = rid;
    return RC::SUCCESSS;
}
