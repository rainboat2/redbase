#include "PF_BufferManager.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define RETURN_ERROR_IF_NOT_PIN(key)            \
    if (fileAllocated_.find(key) == fileAllocated_.end()) { \
        return RC::PF_PAGENOTINBUF;               \
    } else if (unpinDispatcher_->contain(key)) {  \
        return RC::PF_PAGEUNPINNED;               \
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
        // 没有为此页面分配缓存，现在分配一个
        int bufferIndex = getFreeBuffer();
        if (bufferIndex < 0)
            return RC::PF_NOBUF;
        fileAllocated_[key] = std::make_shared<BufferBlockDesc>(fd, pageNum, bufferIndex, 0, false);

        // 读取page数据
        char* buffer = buffer_pool_[bufferIndex];
        PF_UNIX_RETURN_IF_ERROR(lseek(fd, PF_PAGE_OFFSET(pageNum), SEEK_SET));
        int num = read(fd, buffer, PF_FILE_BLOCK_SIZE);
        PF_UNIX_RETURN_IF_ERROR(num);
        if (num < PF_FILE_BLOCK_SIZE)
            return RC::PF_INCOMPLETEREAD;
    }

    auto desc = fileAllocated_[key];
    data = buffer_pool_[desc->bufferIndex];
    pinPage(key);
    return RC::PF_SUCCESSS;
}

RC PF_BufferManager::MarkDirty(int fd, PageNum PageNum)
{
    BufferKey key { fd, PageNum };
    RETURN_ERROR_IF_NOT_PIN(key);
    auto desc = fileAllocated_[key];
    desc->isDirty = true;
    return RC::PF_SUCCESSS;
}

RC PF_BufferManager::ForcePage(int fd, PageNum pageNum)
{
    BufferKey key { fd, pageNum };
    RETURN_ERROR_IF_NOT_PIN(key);
    auto desc = fileAllocated_[key];
    if (desc->isDirty) {
        char* data = buffer_pool_[desc->bufferIndex];
        PF_UNIX_RETURN_IF_ERROR(lseek(fd, PF_PAGE_OFFSET(pageNum), SEEK_SET));
        int num = write(fd, data, PF_FILE_BLOCK_SIZE);
        PF_UNIX_RETURN_IF_ERROR(num);
        if (num < PF_FILE_BLOCK_SIZE)
            return RC::PF_INCOMPLETEWRITE;
        desc->isDirty = false;
    }
    return RC::PF_SUCCESSS;
}


RC PF_BufferManager::UnpinPage(int fd, PageNum pageNum){
    BufferKey key {fd, pageNum};
    RETURN_ERROR_IF_NOT_PIN(key);
    auto desc = fileAllocated_[key];
    desc->pinCount--;
    if (desc->pinCount == 0)
        unpinDispatcher_->push(key);
    return RC::PF_SUCCESSS;
}

RC PF_BufferManager::AllocateBlock(char *&data){
    int bufferIndex = getFreeBuffer();
    if (bufferIndex < 0)
        return RC::PF_NOBUF;
    data = buffer_pool_[bufferIndex];
    scratchAllocated_[data] = bufferIndex;
    return RC::PF_SUCCESSS;
}

RC PF_BufferManager::DisposeBlock(char *&data){
    if (scratchAllocated_.find(data) == scratchAllocated_.end())
        return RC::PF_PAGENOTINBUF;
    int bufferIndex = scratchAllocated_[data];
    scratchAllocated_.erase(data);
    freeBuffers_.push_back(bufferIndex);
    return RC::PF_SUCCESSS;
}

void PF_BufferManager::pinPage(BufferKey& key)
{
    auto desc = fileAllocated_[key];
    if (desc->pinCount == 0) {
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
        // 基于调度策略，淘汰unpin的page，腾出空间
        const auto key = unpinDispatcher_->pop();
        const auto desc = fileAllocated_[key];
        rs = desc->fd;
        fileAllocated_.erase(key);
    }
    return rs;
}