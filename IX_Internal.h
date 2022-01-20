#ifndef IX_INTERNAL_HH
#define IX_INTERNAL_HH

#include <functional>

#include "pf.h"
#include "redbase.h"
#include "rm.h"

struct IX_BFileHeader {
    int attrLength;
    AttrType attrType;
    PageNum pageNums;
    int height;
    int order;
    RID root;
};

// node: [size, attr0, ...,  attrN, rid0, ..., ridN+1]
class IX_BNodeWapper {
public:
    IX_BNodeWapper() = default;
    IX_BNodeWapper(int attrLength, AttrType attrType_, char* data);
    ~IX_BNodeWapper() = default;

    inline int size() const { return *size_; };
    inline bool isFull() const { return *size_ == order_; }

    inline RID getAddr(int i) const {return rids_[i];}
    inline void* getData(int i) const {return (char*)attrs_ + attrLength_ * i;}

    // return index of pdata in bnode, if pdata not in bnode, return the last index that it's element less than pdata.
    int indexOf(const void* pData) const;

public:
    static void initNode(PF_PageHandle& page);
    static constexpr inline int countOrder(int attrLength)
    {
        // sizeof(BNode) = sizeof(int) + order * (attrLength) + (order + 1) * sizeof(rid) < PF_PAGE_SIZE
        return (PF_PAGE_SIZE - sizeof(unsigned int) - sizeof(RID)) / (attrLength + sizeof(RID));
    }

private:
    int attrLength_;
    AttrType attrType_;
    char* raw_data_;
    int order_;

    int* size_;
    RID* rids_;
    void* attrs_;
    std::function<int(const void* d1, const void* d2)> cmp_;
};

#endif