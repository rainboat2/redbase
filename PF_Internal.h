#ifndef PF_INTERNAL_HH
#define PF_INTERNAL_HH

#include <cstdint>
#include <errno.h>
#include <iostream>
#include <string.h>

using PageNum = int32_t;

#define ALL_PAGES -1
#define PF_PAGE_SIZE 4092
#define PF_FILE_BLOCK_SIZE (PF_PAGE_SIZE + 4)
#define PF_BUFFER_SIZE 40
#define PF_FILE_HEADER_SIZE 4096

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
    int pageNum;
};

enum class RC {
    PF_SUCCESSS = 0,
    PF_EOF = 1, // end of file
    PF_PAGEPINNED, // page pinned in buffer
    PF_PAGENOTINBUF, // page to be unpinned is not in buffer
    PF_PAGEUNPINNED, // page already unpinned
    PF_PAGEFREE, // page already free
    PF_INVALIDPAGE, // invalid page number
    PF_FILEOPEN, // file handle already open
    PF_CLOSEDFILE, // file is closed

    PF_NOMEM = -1000, // out of memory
    PF_NOBUF, // out of buffer space
    PF_INCOMPLETEREAD, // incomplete read of page from file
    PF_INCOMPLETEWRITE, // incomplete write of page to file
    PF_HDRREAD, // incomplete read of header from file
    PF_HDRWRITE, // incomplete write of header to file

    // Internal PF errors:
    PF_PAGEINBUF, // new allocated page already in buffer
    PF_HASHNOTFOUND, // hash table entry not found
    PF_HASHPAGEEXIST, // page already exists in hash table
    PF_INVALIDNAME, // invalid file name
    PF_UNIX, // Unix error
};

#define PF_UNIX_RETURN_IF_ERROR(sys_lib_call_or_return_value) \
    {                                                         \
        int rv = sys_lib_call_or_return_value;                \
        if (rv < 0) {                                         \
            std::cout << strerror(errno) << std::endl;        \
            return RC::PF_UNIX;                               \
        }                                                     \
    }

#define RETURN_CODE_IF_NOT_SUCCESS(rc) \
    {                                  \
        if (rc != RC::PF_SUCCESSS)     \
            return rc;                 \
    }

#define PF_PAGE_OFFSET(pageNum) (pageNum * PF_FILE_BLOCK_SIZE + PF_FILE_HEADER_SIZE)
#endif