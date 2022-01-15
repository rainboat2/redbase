#ifndef REDBASE_HH
#define REDBASE_HH

#define MAX_STRING_LEN 255

// definition of return code
enum class RC {
    SUCCESSS = 0,
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

void PrintError(RC rc);

enum class AttrType{
    RD_INT, RD_FLOAT, RD_STRING
};

enum class CompOp{
    EQ, LT, GT, LE, GE, NE, NO
};

enum class ClientHint{
    NO_HINT
};

#define RETURN_CODE_IF_NOT_SUCCESS(redbase_call_or_rc) \
    {                                             \
        RC rc = redbase_call_or_rc;                    \
        if (rc != RC::SUCCESSS)                \
            return rc;                            \
    }

#endif