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
    (RM_PageHeader::size(fileHeader_.recordNumsOfEachPage) + slotNum * fileHeader_.recordSize)

RM_FileHandle::RM_FileHandle()
    : isOpen_(false)
{
}

RM_FileHandle::~RM_FileHandle()
{
    if (isOpen_) {
        ForcePages(ALL_PAGES);
        if (isHeaderChange_) {
            ForceHeader();
        }
    }
}

RC RM_FileHandle::GetRec(const RID& rid, RM_Record& rec) const
{
    if (!isOpen_)
        return RC::RM_FILE_CLOSED;
    RETURN_IF_INVALID_RID(rid);

    EXTRACT_PAGE_SLOT_NUM(pageNum, slotNum, rid);

    PF_PageHandle page;
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(pageNum, page));

    char* data;
    page.GetData(data);
    RM_PageHeader* pageHdr = (RM_PageHeader*)data;
    BitMapWapper bitmap(&pageHdr->bitmap, fileHeader_.recordNumsOfEachPage);

    if (bitmap.get(slotNum) == false) {
        RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(pageNum));
        return RC::RM_EMPTY_SLOT;
    } else {
        rec.setData(data + SLOT_OFFSET(slotNum), fileHeader_.recordSize);
        rec.rid_ = rid;
        RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(pageNum));
        return RC::SUCCESS;
    }
}

RC RM_FileHandle::GetNextRec(const RID& rid, RM_Record& rec) const
{
    EXTRACT_PAGE_SLOT_NUM(pageNum, slotNum, rid);

    bool find = false;
    while (!find && pageNum < fileHeader_.pageNums) {
        while (!find && ++slotNum < fileHeader_.recordNumsOfEachPage) {
            RID next(pageNum, slotNum);
            RC rc = GetRec(next, rec);
            if (rc == RC::SUCCESS) {
                rec.rid_ = next;
                find = true;
            } else if (rc == RC::RM_EMPTY_SLOT) {
                find = false;
            } else {
                return rc;
            }
        }
        pageNum++;
        slotNum = -1;
    }
    if (find)
        return RC::SUCCESS;
    else
        return RC::RM_FILE_EOF;
}

RC RM_FileHandle::InsertRec(const char* pData, RID& rid)
{
    RETURN_RC_IF_NOT_SUCCESS(getFreeSlot(rid));
    RETURN_RC_IF_NOT_SUCCESS(writeDataToSlot(pData, rid));
    isHeaderChange_ = true;
    return RC::SUCCESS;
}

RC RM_FileHandle::DeleteRec(const RID& rid)
{
    EXTRACT_PAGE_SLOT_NUM(pageNum, slotNum, rid);

    // getPageData
    PF_PageHandle page;
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(pageNum, page));
    char* data;
    page.GetData(data);

    BitMapWapper bitmap(&((RM_PageHeader*)data)->bitmap, fileHeader_.recordNumsOfEachPage);
    if (bitmap.all()) {
        RETURN_RC_IF_NOT_SUCCESS(MarkPageAsNotFull(page));
    }
    bitmap.set(slotNum, false);
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(pageNum));
    isHeaderChange_ = true;
    return RC::SUCCESS;
}

RC RM_FileHandle::UpdateRec(const RM_Record& rec)
{
    RID rid;
    rec.GetRid(rid);
    char* data;
    rec.GetData(data);
    RETURN_RC_IF_NOT_SUCCESS(writeDataToSlot(data, rid));
    return RC::SUCCESS;
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
        RETURN_RC_IF_NOT_SUCCESS(AllocateNewPage(page));
    } else {
        RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(fileHeader_.nextFree, page));
    }
    page.GetPageNum(pageNum);

    // get free slot
    char* data;
    page.GetData(data);
    RM_PageHeader* pageHdr = (RM_PageHeader*)data;
    BitMapWapper bitmap(&pageHdr->bitmap, fileHeader_.recordNumsOfEachPage);
    SlotNum slotNum = bitmap.findFirstZero();
    
    assert(slotNum != -1);
    bitmap.set(slotNum, true);
    if (bitmap.all())
        MarkPageAsFull(page);

    page.GetPageNum(rid.pageNum_);
    rid.slotNum_ = slotNum;

    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(pageNum));
    return RC::SUCCESS;
}

RC RM_FileHandle::writeDataToSlot(const char* pData, RID& rid)
{
    // get page
    PF_PageHandle page;
    PageNum pageNum;
    rid.GetPageNum(pageNum);
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(pageNum, page));

    // write data to page
    char* data;
    page.GetData(data);
    SlotNum slotNum;
    rid.GetSlotNum(slotNum);
    memcpy(data + SLOT_OFFSET(slotNum), pData, fileHeader_.recordSize);
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    pf_fileHandle_.UnpinPage(pageNum);
    return RC::SUCCESS;
}

RC RM_FileHandle::AllocateNewPage(PF_PageHandle& page)
{
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.AllocatePage(page));

    char* data;
    page.GetData(data);
    memset(data, 0, PF_PAGE_SIZE);
    MarkPageAsNotFull(page);

    PageNum pageNum;
    page.GetPageNum(pageNum);
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    fileHeader_.pageNums++;
    isHeaderChange_ = true;
    return RC::SUCCESS;
}

// push into stack, non-reentrant
RC RM_FileHandle::MarkPageAsNotFull(PF_PageHandle& page)
{
    char* data;
    page.GetData(data);
    RM_PageHeader* pageHdr = (RM_PageHeader*)data;
    // no need to mark if page alread not full
    assert(!(pageHdr->nextFree == PageStatus::LIST_END || pageHdr->nextFree > 0));

    pageHdr->nextFree = fileHeader_.nextFree;

    PageNum pageNum;
    page.GetPageNum(pageNum);
    fileHeader_.nextFree = pageNum;

    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    isHeaderChange_ = true;
    return RC::SUCCESS;
}

// pop out from stack, non-reentrant
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
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(pageNum));
    return RC::SUCCESS;
}

RC RM_FileHandle::ForceHeader()
{
    if (!isHeaderChange_)
        return RC::SUCCESS;
    PF_PageHandle page;
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.GetFirstPage(page));
    char* data;
    page.GetData(data);
    memcpy(data, &fileHeader_, sizeof(RM_FileHeader));
    isHeaderChange_ = false;

    PageNum pageNum;
    page.GetPageNum(pageNum);
    pf_fileHandle_.MarkDirty(pageNum);
    pf_fileHandle_.UnpinPage(pageNum);
    return RC::SUCCESS;
}