#ifndef IX_INTERNAL_HH
#define IX_INTERNAL_HH

#include "pf.h"
#include "rm.h"
#include "redbase.h"

struct IX_BFileHeader
{
    int attrLength;
    AttrType attrType;
    PageNum pageNums;
    int height;
    int order;
    RID root;
};

// node: [size, attr0, ...,  attrN, rid0, ..., ridN+1]
class IX_BNodeWapper{
public:
    IX_BNodeWapper(int attrSize, AttrType attrType_, char *data, int order);
    ~IX_BNodeWapper() = default;
    static void init_node(PF_PageHandle &page);
};


#endif