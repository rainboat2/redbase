#include "RM_Internal.h"
#include "redbase.h"
#include "rm.h"

#include <cassert>

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

#define SLOT_OFFSET(slotNum) \
    (RM_PageHeader::size(fileHeader_.recordSize) + slotNum * fileHeader_.recordSize)

RM_FileHandle::RM_FileHandle()
    : isOpen(false)
{
}

RM_FileHandle::~RM_FileHandle()
{
    ForcePages(ALL_PAGES);
    if (isHeaderChange_){
        ForceHeader();
    }
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

    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(pageNum));
    return RC::SUCCESSS;
}

RC RM_FileHandle::InsertRec(const char* pData, RID& rid)
{
    RETURN_CODE_IF_NOT_SUCCESS(getFreeSlot(rid));
    RETURN_CODE_IF_NOT_SUCCESS(writeDataToSlot(pData, rid));
    return RC::SUCCESSS;
}

RC RM_FileHandle::UpdateRec(const RM_Record& rec)
{
    RID rid;
    rec.GetRid(rid);
    char* data;
    rec.GetData(data);
    RETURN_CODE_IF_NOT_SUCCESS(writeDataToSlot(data, rid));
    return RC::SUCCESSS;
}

RC RM_FileHandle::ForcePages(PageNum pageNum) const
{
    return pf_fileHandle_.ForcePages(pageNum);
}

RC RM_FileHandle::getFreeSlot(RID& rid)
{
    PageNum pageNum;
    PF_PageHandle page;
    if (fileHeader_.nextFree == PageStatus::LIST_END) {
        RETURN_CODE_IF_NOT_SUCCESS(AllocateNewPage(page));
    } else {
        RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(fileHeader_.nextFree, page));
    }
    page.GetPageNum(pageNum);

    // get free slot
    char* data;
    page.GetData(data);
    RM_PageHeader* pageHdr = (RM_PageHeader*)data;
    BitMapWapper bitmap(pageHdr->bitmap, fileHeader_.recordNumsOfEachPage);
    SlotNum slotNum = bitmap.findFirstZero();
    bitmap.set(slotNum, true);
    if (bitmap.all())
        MarkPageAsFull(page);

    page.GetPageNum(rid.pageNum_);
    rid.slotNum_ = slotNum;

    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(pageNum));
    return RC::SUCCESSS;
}

RC RM_FileHandle::writeDataToSlot(const char* pData, RID& rid)
{
    // get page
    PF_PageHandle page;
    PageNum pageNum;
    rid.GetPageNum(pageNum);
    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(pageNum, page));

    // write data to page
    char* data;
    page.GetData(data);
    SlotNum slotNum;
    rid.GetSlotNum(slotNum);
    memcpy(data + SLOT_OFFSET(slotNum), pData, fileHeader_.recordSize);
    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    pf_fileHandle_.UnpinPage(pageNum);
    return RC::SUCCESSS;
}

RC RM_FileHandle::AllocateNewPage(PF_PageHandle& page)
{
    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.AllocatePage(page));

    char* data;
    page.GetData(data);
    memset(data, 0, PF_PAGE_SIZE);
    MarkPageAsNotFull(page);

    PageNum pageNum;
    page.GetPageNum(pageNum);
    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    return RC::SUCCESSS;
}

// push into stack
RC RM_FileHandle::MarkPageAsNotFull(PF_PageHandle& page)
{
    char* data;
    page.GetData(data);
    RM_PageHeader* pageHdr = (RM_PageHeader*)data;
    pageHdr->nextFree = fileHeader_.nextFree;

    PageNum pageNum;
    page.GetPageNum(pageNum);
    fileHeader_.nextFree = pageNum;

    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    isHeaderChange_ = true;
    return RC::SUCCESSS;
}

// pop out from stack
RC RM_FileHandle::MarkPageAsFull(PF_PageHandle& page)
{
    char* data;
    page.GetData(data);
    RM_PageHeader* pageHdr = (RM_PageHeader*)data;

    PageNum pageNum;
    page.GetPageNum(pageNum);

    // only free page on stack top can be accessed
    assert(fileHeader_.nextFree == pageNum);
    assert(pageHdr->nextFree != PageStatus::RM_PAGE_FULL);

    fileHeader_.nextFree = pageHdr->nextFree;
    isHeaderChange_ = true;
    pageHdr->nextFree = PageStatus::RM_PAGE_FULL;
    RETURN_CODE_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    return RC::SUCCESSS;
}

RC RM_FileHandle::ForceHeader()
{
    if (!isHeaderChange_)
        return RC::SUCCESSS;
    PF_PageHandle page;
    char *data;
    page.GetData(data);
    memcpy(&fileHeader_, data, sizeof(RM_FileHandle));
    isHeaderChange_ = false;

    PageNum pageNum;
    page.GetPageNum(pageNum);
    pf_fileHandle_.MarkDirty(pageNum);
    return RC::SUCCESSS;
}