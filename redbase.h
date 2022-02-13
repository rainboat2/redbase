#ifndef REDBASE_HH
#define REDBASE_HH

#include <functional>

#define MAX_STRING_LEN 255

enum PageStatus : int {
    LIST_END = -1,
    PF_USED = -2,
    PF_INVALID = -3,

    RM_PAGE_FULL = -4,
};

// definition of return code
enum class RC {
    SUCCESS = 0,
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

    RM_RECORD_SIZE_TOO_LAGRE = 1000,
    RM_INVALID_RECORD_SIZE,
    RM_INVALID_RID,
    RM_EMPTY_SLOT,
    RM_FILE_CLOSED,
    RM_FILE_OPENED,
    RM_FILE_SCAN_OPENED,
    RM_FILE_SCAN_CLOSED,
    RM_FILE_EOF,

    IX_INDEX_OPENED = 2000,
    IX_INDEX_CLOSED,
    IX_INDEX_SCAN_OPENED,
    IX_INDEX_SCAN_CLOSED,
    IX_INDEX_SCAN_EOF,

    SM_ATTR_NOT_FOUND = 3000,
    SM_INDEX_ARLEADY_EXIST,
    SM_DB_OPENED,
    SM_DB_CLOSED,
    SM_UNIX,
};

void PrintError(RC rc);

enum class AttrType : int {
    RD_INT,
    RD_FLOAT,
    RD_STRING
};

enum class CompOp {
    EQ,
    NE,
    GE,
    GT,
    LE,
    LT,
    NO
};

std::function<int(const void* d1, const void* d2)> getComparator(AttrType attrType, int attrLength);

enum class ClientHint {
    NO_HINT
};

#define RETURN_RC_IF_NOT_SUCCESS(redbase_call_or_rc) \
    {                                                \
        RC rc__ = redbase_call_or_rc;                \
        if (rc__ != RC::SUCCESS)                     \
            return rc__;                             \
    }

#define EXIT_IF_NOT_SUCESSS(redbase_call_or_rc)                    \
    {                                                              \
        RC rc__ = redbase_call_or_rc;                              \
        if (rc__ != RC::SUCCESS) {                                 \
            PrintError(rc__);                                      \
            std::cout << __FILE__ << ":" << __LINE__ << std::endl; \
            exit(-1);                                              \
        }                                                          \
    }

#endif