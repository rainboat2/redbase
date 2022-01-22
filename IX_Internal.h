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

struct IX_BInsertUpEntry{
    bool isSpilt = false;
    char* attr = nullptr;
    RID left {-1, -1};
    RID rigth {-1, -1};
};

// node: [size, attr0, ...,  attrN, rid0, ..., ridN+1]
class IX_BNodeWapper {
public:
    IX_BNodeWapper() = default;
    IX_BNodeWapper(int attrLength, AttrType attrType_, char* nodeData, RID addr);
    ~IX_BNodeWapper() = default;

    inline int size() const { return *size_; };
    inline bool isFull() const { return *size_ == order_; }

    inline void setRid(int i, RID rid) { rids_[i] = rid; }
    inline RID getRid(int i) const { return rids_[i]; }
    inline void setAttr(int i, void* data) { memcpy(getAttr(i), data, attrLength_); }
    inline void* getAttr(int i) const { return (char*)attrs_ + attrLength_ * i; }

    inline PageNum getAddrInFile()
    {
        PageNum num;
        addr_.GetPageNum(num);
        return num;
    }

    // return index of pdata in bnode, if pdata not in bnode, return the first index that it's element greater than pdata.
    int indexOf(const void* pData) const;

    // insert a new attr into BNode, return the index of the attr
    int insert(void* attr, RID rid);
    IX_BInsertUpEntry insertAndSpilt(void* attr, RID rid, IX_BNodeWapper &newNode, bool isCopyUp);

public:
    static void initNode(PF_PageHandle& page);
    static constexpr inline int countOrder(int attrLength)
    {
        // sizeof(BNode) = sizeof(int) + order * (attrLength) + (order + 1) * sizeof(rid) < PF_PAGE_SIZE
        return (PF_PAGE_SIZE - sizeof(int) - sizeof(RID)) / (attrLength + sizeof(RID));
    }

private:
    void swap(int i, int j);
    void spiltInto(IX_BNodeWapper &other, IX_BInsertUpEntry &up);

private:
    // data used for assistance
    int attrLength_;
    AttrType attrType_;
    int order_;
    RID addr_;
    std::function<int(const void* d1, const void* d2)> cmp_;

    // data of bNode
    char* raw_data_;

    // pointer to data of bNode
    int* size_;
    RID* rids_;
    void* attrs_;
};

#endif