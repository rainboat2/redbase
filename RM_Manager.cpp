#include "RM_Internal.h"
#include "rm.h"

RM_Manager::RM_Manager(PF_Manager& pf)
    : pf_manager_(pf)
{
}

RC RM_Manager::CreateFile(const char* fileName, int recordSize)
{
    if (recordSize <= 0) {
        return RC::RM_INVALID_RECORD_SIZE;
    }
    int recordNums = countRecordNums(recordSize);
    if (recordNums <= 0) {
        return RC::RM_RECORD_SIZE_TOO_LAGRE;
    }

    RETURN_CODE_IF_NOT_SUCCESS(pf_manager_.CreateFile(fileName));
    PF_FileHandle pfh;
    RETURN_CODE_IF_NOT_SUCCESS(pf_manager_.OpenFile(fileName, pfh));
    PF_PageHandle page;
    RETURN_CODE_IF_NOT_SUCCESS(pfh.AllocatePage(page));
    char* data;
    page.GetData(data);
    RM_FileHeader* rfh = (RM_FileHeader*)data;
    rfh->pageNums = 0;
    rfh->nextFree = PageStatus::LIST_END;
    rfh->recordSize = recordSize;
    rfh->recordNumsOfEachPage = recordNums;
    pfh.MarkDirty(0);
    pfh.UnpinPage(0);
    return RC::SUCCESSS;
}

RC RM_Manager::DestroyFile(const char* fileName)
{
    return pf_manager_.DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char* fileName, RM_FileHandle& fileHandle)
{
    if (fileHandle.isOpen)
        return RC::RM_FILE_ALREAD_OPEN;

    RETURN_CODE_IF_NOT_SUCCESS(pf_manager_.OpenFile(fileName, fileHandle.pf_fileHandle_));
    PF_PageHandle page;
    RETURN_CODE_IF_NOT_SUCCESS(fileHandle.pf_fileHandle_.GetFirstPage(page));
    fileHandle.isOpen = true;
    char *data;
    page.GetData(data);
    fileHandle.fileHeader_ = *((RM_FileHeader*) data);
    fileHandle.isHeaderChange_ = false;

    PageNum pageNum;
    page.GetPageNum(pageNum);
    RETURN_CODE_IF_NOT_SUCCESS(fileHandle.pf_fileHandle_.UnpinPage(pageNum));
    return RC::SUCCESSS;
}

RC RM_Manager::CloseFile(RM_FileHandle& fileHandle)
{
    fileHandle.ForcePages(ALL_PAGES);
    fileHandle.ForceHeader();
    RETURN_CODE_IF_NOT_SUCCESS(pf_manager_.CloseFile(fileHandle.pf_fileHandle_));
    fileHandle.isOpen = false;
    return RC::SUCCESSS;
}

// x := recordNums
// x := ( PF_PAGE_SIZE - sizeof(int) - BITMAP_SIZE(x)) / recordSize
int RM_Manager::countRecordNums(int recordSize)
{
    int recordNums = (PF_PAGE_SIZE - sizeof(int)) / recordSize;
    while (recordNums > 0 && (RM_PageHeader::size(recordNums) + recordNums * recordSize) > PF_PAGE_SIZE)
        recordNums--;
    return recordNums;
}