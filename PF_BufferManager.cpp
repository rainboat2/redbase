#include "PF_BufferManager.h"
#include "PF_Internal.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define RETURN_ERROR_IF_NOT_PIN(key)                        \
    if (fileAllocated_.find(key) == fileAllocated_.end()) { \
        return RC::PF_PAGENOTINBUF;                         \
    } else if (unpinDispatcher_->contain(key)) {            \
        return RC::PF_PAGEUNPINNED;                         \
    }

PF_BufferManager::PF_BufferManager()
{
    freeBuffers_.reserve(PF_BUFFER_SIZE);
    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        buffer_pool_[i] = new char[PF_FILE_BLOCK_SIZE];
        freeBuffers_.push_back(i);
    }
    unpinDispatcher_ = new LRU<BufferKey, BufferKeyHash>;
}

PF_BufferManager::~PF_BufferManager()
{
    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        delete[] buffer_pool_[i];
    }
    delete unpinDispatcher_;
}

RC PF_BufferManager::ReadPage(int fd, PageNum pageNum, char*& data)
{
    BufferKey key { fd, pageNum };

    if (fileAllocated_.find(key) == fileAllocated_.end()) {
        // if page data not in buffer, allocate buffer for this page
        int bufferIndex = getFreeBuffer();
        if (bufferIndex < 0)
            return RC::PF_NOBUF;
        fileAllocated_[key] = std::make_shared<BufferBlockDesc>(fd, pageNum, bufferIndex, 0, false);

        char* buffer = buffer_pool_[bufferIndex];
        RETURN_RC_IF_NOT_SUCCESS(ReadPageFromFile(fd, pageNum, buffer));
    }

    auto desc = fileAllocated_[key];
    data = buffer_pool_[desc->bufferIndex];
    pinPage(key);
    return RC::SUCCESS;
}

RC PF_BufferManager::MarkDirty(int fd, PageNum pageNum)
{
    BufferKey key { fd, pageNum };
    RETURN_ERROR_IF_NOT_PIN(key);
    auto desc = fileAllocated_[key];
    desc->isDirty = true;
    return RC::SUCCESS;
}

RC PF_BufferManager::ForcePage(int fd, PageNum pageNum)
{
    BufferKey key { fd, pageNum };
    RETURN_ERROR_IF_NOT_PIN(key);
    auto desc = fileAllocated_[key];
    if (desc->isDirty) {
        char* data = buffer_pool_[desc->bufferIndex];
        WritePageToFile(fd, pageNum, data);
        desc->isDirty = false;
    }
    return RC::SUCCESS;
}

RC PF_BufferManager::UnpinPage(int fd, PageNum pageNum)
{
    BufferKey key { fd, pageNum };
    RETURN_ERROR_IF_NOT_PIN(key);
    auto desc = fileAllocated_[key];
    desc->pinCount--;
    if (desc->pinCount == 0) {
        unpinDispatcher_->push(key);
    }
    return RC::SUCCESS;
}

RC PF_BufferManager::AllocateBlock(char*& data)
{
    int bufferIndex = getFreeBuffer();
    if (bufferIndex < 0)
        return RC::PF_NOBUF;
    data = buffer_pool_[bufferIndex];
    scratchAllocated_[data] = bufferIndex;
    return RC::SUCCESS;
}

RC PF_BufferManager::DisposeBlock(char*& data)
{
    if (scratchAllocated_.find(data) == scratchAllocated_.end())
        return RC::PF_PAGENOTINBUF;
    int bufferIndex = scratchAllocated_[data];
    scratchAllocated_.erase(data);
    freeBuffers_.push_back(bufferIndex);
    return RC::SUCCESS;
}

void PF_BufferManager::pinPage(BufferKey& key)
{
    auto desc = fileAllocated_[key];
    if (desc->pinCount == 0 && unpinDispatcher_->contain(key)) {
        unpinDispatcher_->remove(key);
    }
    desc->pinCount++;
}

int PF_BufferManager::getFreeBuffer()
{
    int rs = -1;

    if (!freeBuffers_.empty()) {
        rs = freeBuffers_.back();
        freeBuffers_.pop_back();
    } else if (!unpinDispatcher_->empty()) {
        // based on buffer strategy, swap out the page
        const auto key = unpinDispatcher_->pop();
        const auto desc = fileAllocated_[key];
        ForcePage(desc->fd, desc->pageNum);
        rs = desc->bufferIndex;
        fileAllocated_.erase(key);
    }
    return rs;
}

RC PF_BufferManager::ReadPageFromFile(int fd, PageNum pageNum, char*& data)
{
    PF_UNIX_RETURN_IF_ERROR(lseek(fd, PF_PAGE_OFFSET(pageNum), SEEK_SET));
    int num = read(fd, data, PF_FILE_BLOCK_SIZE);
    PF_UNIX_RETURN_IF_ERROR(num);
    if (num < PF_FILE_BLOCK_SIZE)
        return RC::PF_INCOMPLETEREAD;
    return RC::SUCCESS;
}

RC PF_BufferManager::WritePageToFile(int fd, PageNum pageNum, char* data)
{

    PF_UNIX_RETURN_IF_ERROR(lseek(fd, PF_PAGE_OFFSET(pageNum), SEEK_SET));
    int num = write(fd, data, PF_FILE_BLOCK_SIZE);
    PF_UNIX_RETURN_IF_ERROR(num);
    if (num < PF_FILE_BLOCK_SIZE)
        return RC::PF_INCOMPLETEWRITE;
    return RC::SUCCESS;
}