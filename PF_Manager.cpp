#include "PF_Internal.h"
#include "pf.h"

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

PF_Manager::PF_Manager()
    : buffer_manger_(new PF_BufferManager())
{
}

PF_Manager::~PF_Manager()
{
    delete buffer_manger_;
}

RC PF_Manager::CreateFile(const char* filename)
{
    int fd = open(filename, O_RDWR | O_CREAT | O_EXCL, 0644);
    PF_UNIX_RETURN_IF_ERROR(fd);

    // header虽然不大，但使用一个Block大小来存储header
    char buffer[PF_FILE_HEADER_SIZE];
    PF_FileHeader* header = (PF_FileHeader*)buffer;
    header->nextFree = -1;
    header->pageNum = 1;

    auto num = write(fd, header, PF_FILE_HEADER_SIZE);
    PF_UNIX_RETURN_IF_ERROR(num);
    if (num >= 0 && num < PF_FILE_HEADER_SIZE) {
        return RC::PF_INCOMPLETEWRITE;
    }

    PF_UNIX_RETURN_IF_ERROR(close(fd));
    return RC::PF_SUCCESSS;
}

RC PF_Manager::DestroyFile(const char* filename)
{
    PF_UNIX_RETURN_IF_ERROR(unlink(filename));
    return RC::PF_SUCCESSS;
}

RC PF_Manager::OpenFile(const char* filename, PF_FileHandle& fileHandle)
{
    if (fileHandle.isOpen_)
        return RC::PF_FILEOPEN;

    int fd = open(filename, O_RDWR);
    PF_UNIX_RETURN_IF_ERROR(fd);

    fileHandle.filename_ = filename;
    fileHandle.isOpen_ = true;
    fileHandle.fd_ = fd;
    fileHandle.isHeadChange_ = false;
    fileHandle.bufferManager_ = buffer_manger_;

    char buffer[PF_FILE_BLOCK_SIZE];
    PF_UNIX_RETURN_IF_ERROR(lseek(fd, 0, SEEK_SET));
    int num = read(fd, buffer, PF_FILE_BLOCK_SIZE);
    PF_UNIX_RETURN_IF_ERROR(num);
    if (num < PF_FILE_BLOCK_SIZE)
        return RC::PF_INCOMPLETEREAD;
    fileHandle.header_ = *((PF_FileHeader*)buffer);
    return RC::PF_SUCCESSS;
}

RC PF_Manager::CloseFile(PF_FileHandle& fileHandle)
{
    fileHandle.ForcePages(ALL_PAGES);
    fileHandle.ForceHeader();
    fileHandle.isOpen_ = false;
    fileHandle.filename_ = nullptr;
    PF_UNIX_RETURN_IF_ERROR(close(fileHandle.fd_));
    fileHandle.fd_ = -1;
    return RC::PF_SUCCESSS;
}

RC PF_Manager::AllocateBlock(char*& buffer)
{
    return buffer_manger_->AllocateBlock(buffer);
}

RC PF_Manager::DisposeBlock(char* buffer)
{
    return buffer_manger_->DisposeBlock(buffer);
}