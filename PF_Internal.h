#ifndef PF_INTERNAL_HH
#define PF_INTERNAL_HH

#include <errno.h>
#include <iostream>
#include <string.h>

using PageNum = int;
#define PF_FILE_BLOCK_SIZE 4096
#define PF_BUFFER_SIZE 40
#define PF_FILE_HEADER_SIZE 4096
#define PF_PAGE_OFFSET(pageNum) (pageNum * PF_FILE_BLOCK_SIZE + PF_FILE_HEADER_SIZE)

enum PageStatus : int {
    LIST_END = -1,
    USED = -2,
    INVALID = -3
};

struct PF_PageHeader {
    // 大于0的时候表示下一个free page，小于0表示page的状态
    int nextFree;
};

struct PF_FileHeader {
    int nextFree;
    PageNum pageNum;
};


#define PF_UNIX_RETURN_IF_ERROR(sys_lib_call_or_return_value) \
    {                                                         \
        int rv = sys_lib_call_or_return_value;                \
        if (rv < 0) {                                         \
            std::cout << strerror(errno) << std::endl;        \
            return RC::PF_UNIX;                               \
        }                                                     \
    }


#endif