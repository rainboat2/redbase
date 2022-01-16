#include "pf.h"
#include "PF_BufferManager.h"
#include "PF_BufferStrategy.h"

#include <unordered_map>
#include <iostream>

// struct BufferBlockDesc
// {
//     int fd;
//     PageNum pageNum;
//     char* data;
//     short int pinCount;
//     bool isDirty;
// };

int main(){
    const char* file = "/tmp/test";
    PF_Manager manger;
    PF_FileHandle fileHandle;
    manger.CreateFile(file);
    manger.OpenFile(file, fileHandle);
    PF_PageHandle pageHandle;
    fileHandle.AllocatePage(pageHandle);

    char *data;
    PageNum pageNum;
    pageHandle.GetData(data);
    pageHandle.GetPageNum(pageNum);
    memset(data, 'a', PF_PAGE_SIZE);
    fileHandle.MarkDirty(pageNum);
    fileHandle.UnpinPage(pageNum);
    return 0;
}

