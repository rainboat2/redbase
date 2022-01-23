#include <iostream>
#include <string>
#include <unordered_map>

#include "redbase.h"

void PrintError(RC rc)
{
    static std::unordered_map<RC, const char*> m = {
        { RC::SUCCESSS, "successs!" },
        { RC::PF_EOF, "end of file!" },
        { RC::PF_PAGEPINNED, "page pinned in buffer!" },
        { RC::PF_PAGENOTINBUF, "page to be unpinned is not in buffer!" },
        { RC::PF_PAGEUNPINNED, "page already unpinned!" },
        { RC::PF_PAGEFREE, "page already free!" },
        { RC::PF_INVALIDPAGE, "invalid page number!" },
        { RC::PF_FILEOPEN, "file handle already open!" },
        { RC::PF_CLOSEDFILE, "file is close!" },
        { RC::PF_NOMEM, "out of memory!" },
        { RC::PF_NOBUF, "out of buffer space!" },
        { RC::PF_INCOMPLETEREAD, "incomplete read of page from file!" },
        { RC::PF_INCOMPLETEWRITE, "incomplete write of page to file!" },
        { RC::PF_HDRREAD, "incomplete read of header from file!" },
        { RC::PF_HDRWRITE, "incomplete write of header to file!" },
        { RC::PF_PAGEINBUF, "new allocated page already in buffer!" },
        { RC::PF_HASHNOTFOUND, "hash table entry not found!" },
        { RC::PF_HASHPAGEEXIST, "page already exists in hash table!" },
        { RC::PF_INVALIDNAME, "invalid file name!" },
        { RC::PF_UNIX, "Unix error!" },
        { RC::RM_RECORD_SIZE_TOO_LAGRE, "record size is too lager!" },
        { RC::RM_INVALID_RECORD_SIZE, "record size if invalid!" },
        { RC::RM_INVALID_RID, "RID is invalid!" },
        { RC::RM_EMPTY_SLOT, "the slot be founded is empty!" },
        { RC::RM_FILE_CLOSED, "rm file already closed!" },
        { RC::RM_FILE_OPENED, "rm file already opened!" },
        { RC::RM_FILE_SCAN_OPENED, "rm file scan already open!" },
        { RC::RM_FILE_SCAN_CLOSED, "rm file scan already closed!" },
        { RC::RM_FILE_EOF, "reach the end of rm file!" },
        { RC::IX_INDEX_OPENED, "index handle already opened!" },
        { RC::IX_INDEX_CLOSED, "index handle already closed!" }
    };
    std::cerr << m[rc] << std::endl;
}