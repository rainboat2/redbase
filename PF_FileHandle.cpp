#include "pf.h"
#include <sys/types.h>
#include <unistd.h>

#define RETURN_CODE_IF_FILE_STATUS_INVALID \
    if (!isOpen_) {                        \
        return RC::PF_CLOSEDFILE;          \
    }

#define RETURN_CODE_IF_PAGE_NUM_INVALID(pageNum)    \
    if (pageNum < 0 || pageNum > header_.pageNum) { \
        return RC::PF_EOF;                          \
    }

PF_FileHandle::PF_FileHandle()
    : filename_(nullptr)
    , fd_(-1)
    , isOpen_(false)
    , isHeadChange_(false)
    , bufferManager_(nullptr)
{
}

PF_FileHandle::~PF_FileHandle(){
    ForcePages(ALL_PAGES);
    if (isHeadChange_)
        ForceHeader();
}

RC PF_FileHandle::GetFirstPage(PF_PageHandle& pageHandle) const
{
    return GetNextPage(-1, pageHandle);
}

RC PF_FileHandle::GetLastPage(PF_PageHandle& pageHandle) const
{
    return GetPrevPage(header_.pageNum, pageHandle);
}

RC PF_FileHandle::GetNextPage(PageNum current, PF_PageHandle& pageHandle) const
{
    RC rc;
    while (true) {
        rc = GetThisPage(++current, pageHandle);
        // PF_INVALIDPAGE说明读到的page是被删除了的page，应该跳过继续找
        // 其他类型的rc要么说明出错，要么成功找到，都应该直接返回。
        if (rc != RC::PF_INVALIDPAGE)
            break;
    }
    return rc;
}

RC PF_FileHandle::GetPrevPage(PageNum current, PF_PageHandle& pageHandle) const
{
    RC rc;
    while (true) {
        rc = GetThisPage(--current, pageHandle);
        if (rc != RC::PF_INVALIDPAGE)
            break;
    }
    return rc;
}

RC PF_FileHandle::GetThisPage(PageNum pageNum, PF_PageHandle& pageHandle) const
{
    RETURN_CODE_IF_PAGE_NUM_INVALID(pageNum);
    RETURN_CODE_IF_FILE_STATUS_INVALID;
    char* data;
    RETURN_CODE_IF_NOT_SUCCESS(bufferManager_->ReadPage(fd_, pageNum, data));

    PF_PageHeader* header_ = (PF_PageHeader*)data;
    if (header_->nextFree != PageStatus::USED) {
        bufferManager_->UnpinPage(fd_, pageNum);
        return RC::PF_INVALIDPAGE;
    }
    pageHandle.pageNum_ = pageNum;
    pageHandle.data_ = data + sizeof(PF_PageHeader);
    return RC::PF_SUCCESSS;
}

RC PF_FileHandle::AllocatePage(PF_PageHandle& pageHandle)
{
    RETURN_CODE_IF_FILE_STATUS_INVALID;
    PageNum pageNum;

    // 获取空闲的page
    if (header_.nextFree == PageStatus::LIST_END) {
        RETURN_CODE_IF_NOT_SUCCESS(appendFileBlockToEnd());
        pageNum = header_.pageNum - 1;
    } else {
        pageNum = header_.nextFree;
    }

    char* data;
    RETURN_CODE_IF_NOT_SUCCESS(bufferManager_->ReadPage(fd_, pageNum, data));

    pageHandle.data_ = data + sizeof(PF_PageHandle);
    pageHandle.pageNum_ = pageNum;

    if (header_.nextFree != PageStatus::LIST_END) {
        PF_PageHeader* h = (PF_PageHeader*)data;
        header_.nextFree = h->nextFree;
        isHeadChange_ = true;
    }
    return RC::PF_SUCCESSS;
}

RC PF_FileHandle::DisposePage(PageNum pageNum)
{
    RETURN_CODE_IF_FILE_STATUS_INVALID;
    RETURN_CODE_IF_PAGE_NUM_INVALID(pageNum);

    char* data;
    RETURN_CODE_IF_NOT_SUCCESS(bufferManager_->ReadPage(fd_, pageNum, data));
    PF_PageHeader* h = (PF_PageHeader*)data;
    h->nextFree = header_.nextFree;
    header_.nextFree = pageNum;

    isHeadChange_ = true;
    RETURN_CODE_IF_NOT_SUCCESS(bufferManager_->MarkDirty(fd_, pageNum));
    RETURN_CODE_IF_NOT_SUCCESS(bufferManager_->UnpinPage(fd_, pageNum));
    return RC::PF_SUCCESSS;
}

RC PF_FileHandle::MarkDirty(PageNum pageNum) const
{
    RETURN_CODE_IF_FILE_STATUS_INVALID;
    RETURN_CODE_IF_PAGE_NUM_INVALID(pageNum);

    return bufferManager_->MarkDirty(fd_, pageNum);
}

RC PF_FileHandle::ForcePages(PageNum pageNum) const
{
    RETURN_CODE_IF_FILE_STATUS_INVALID;
    RETURN_CODE_IF_PAGE_NUM_INVALID(pageNum);

    if (pageNum == ALL_PAGES) {
        for (int i = 0; i < header_.pageNum; i++) {
            RC rc = bufferManager_->ForcePage(fd_, i);
            if (rc != RC::PF_PAGEINBUF && rc != RC::PF_PAGEUNPINNED && rc != RC::PF_SUCCESSS)
                return rc;
        }
        return RC::PF_SUCCESSS;
    } else {
        return bufferManager_->ForcePage(fd_, pageNum);
    }
}

RC PF_FileHandle::UnpinPage(PageNum pageNum) const
{
    RETURN_CODE_IF_FILE_STATUS_INVALID;
    RETURN_CODE_IF_PAGE_NUM_INVALID(pageNum);

    return bufferManager_->UnpinPage(fd_, pageNum);
}

RC PF_FileHandle::appendFileBlockToEnd()
{
    char buffer[PF_FILE_BLOCK_SIZE];
    PF_PageHeader* h = (PF_PageHeader*)buffer;
    h->nextFree = PageStatus::USED;

    PF_UNIX_RETURN_IF_ERROR(lseek(fd_, 0, SEEK_END));
    int num = write(fd_, buffer, PF_FILE_BLOCK_SIZE);
    PF_UNIX_RETURN_IF_ERROR(num);
    if (num < PF_FILE_BLOCK_SIZE)
        return RC::PF_INCOMPLETEWRITE;

    header_.pageNum++;
    isHeadChange_ = true;
    return RC::PF_SUCCESSS;
}

RC PF_FileHandle::ForceHeader()
{
    if (isHeadChange_) {
        char buffer[PF_FILE_BLOCK_SIZE];
        PF_FileHeader* h = (PF_FileHeader*)buffer;
        h->nextFree = header_.nextFree;
        h->pageNum = header_.pageNum;
        PF_UNIX_RETURN_IF_ERROR(lseek(fd_, 0, SEEK_SET));
        int num = write(fd_, buffer, PF_FILE_BLOCK_SIZE);
        PF_UNIX_RETURN_IF_ERROR(num);
        if (num < PF_FILE_BLOCK_SIZE)
            return RC::PF_INCOMPLETEWRITE;
        isHeadChange_ = false;
    }
    return RC::PF_SUCCESSS;
}