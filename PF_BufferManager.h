#ifndef PF_BUFFERMANGER_HH
#define PF_BUFFERMANGER_HH

#include "redbase.h"
#include "PF_Internal.h"
#include "PF_BufferStrategy.h"

#include <unordered_map>
#include <vector>
#include <memory>

// use (fd, pageNum) to identifiy a buffer block
struct BufferKey {
    int fd;
    PageNum pageNum;

    inline bool operator==(const BufferKey& b) const
    {
        return this->fd == b.fd && this->pageNum == b.pageNum; 
    }
};

struct BufferKeyHash {
    std::size_t operator()(const BufferKey& k) const
    {
        return (k.fd << 16) | k.pageNum;
    }
};

struct BufferBlockDesc {
    int fd;
    PageNum pageNum;
    int bufferIndex;
    short int pinCount;
    bool isDirty;

    BufferBlockDesc(int fd, PageNum pageNum, int bufferIndex, short int pinCount, bool isDirty){
        this->fd = fd;
        this->pageNum = pageNum;
        this->bufferIndex = bufferIndex;
        this->pinCount = pinCount;
        this->isDirty = isDirty;
    }
};


class PF_BufferManager {
public:
    PF_BufferManager();
    ~PF_BufferManager();

    RC ReadPage(int fd, PageNum pageNum, char *&data);
    RC MarkDirty(int fd, PageNum pageNum);
    RC ForcePage(int fd, PageNum pageNum);
    RC UnpinPage(int fd, PageNum pageNum);

    RC AllocateBlock(char*& data);
    RC DisposeBlock(char*& data);

private:
    int getFreeBuffer();
    void pinPage(BufferKey &key);
    RC ReadPageFromFile(int fd, PageNum pageNum, char *&data);
    RC WritePageToFile(int fd, PageNum pageNum, char *data);

private:
    char* buffer_pool_[PF_BUFFER_SIZE];
    std::vector<int> freeBuffers_;
    std::unordered_map<BufferKey, std::shared_ptr<BufferBlockDesc>, BufferKeyHash> fileAllocated_;
    std::unordered_map<char*, int> scratchAllocated_;
    BufferStrategy<BufferKey>* unpinDispatcher_;
};

#endif