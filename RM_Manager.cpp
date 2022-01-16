#include "RM_Internal.h"
#include "rm.h"

RM_Manager::RM_Manager(PF_Manager& pf)
    : pf_manager_(pf)
{
}

RC RM_Manager::CreateFile(const char* fileName, int recordSize)
{
    if (recordSize >= (PF_PAGE_SIZE - sizeof(RM_PageHeader) - 4)) {
        return RC::RM_RECORD_SIZE_TOO_LAGRE;
    } else if (recordSize <= 0) {
        return RC::RM_INVALID_RECORD_SIZE;
    }
    RETURN_CODE_IF_NOT_SUCCESS(pf_manager_.CreateFile(fileName));
    PF_FileHandle pfh;
    RETURN_CODE_IF_NOT_SUCCESS(pf_manager_.OpenFile(fileName, pfh));
    PF_PageHandle page;
    RETURN_CODE_IF_NOT_SUCCESS(pfh.AllocatePage(page));
    char *data;
    page.GetData(data);
    RM_FileHeader *rfh = (RM_FileHeader*) data;
    rfh->pageNums = 0;
    rfh->nextFree = PageStatus::USED;
    rfh->recordSize = recordSize;
    // x := recordNums
    // x := ( PF_PAGE_SIZE - sizeof(int) - BITMAP_SIZE(x)) / recordSize
    rfh->recordNumsOfEachPage = PF_PAGE_SIZE - sizeof(int);
    return RC::SUCCESSS;
}