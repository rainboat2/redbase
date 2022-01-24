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

struct IX_BInsertUpEntry {
    bool isSpilt = false;
    char attr[MAX_STRING_LEN];
    RID right { -1, -1 };
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
    inline void* getAttr(int i) const { return attrs_ + attrLength_ * i; }

    inline PageNum getPageNum()
    {
        PageNum num;
        addr_.GetPageNum(num);
        return num;
    }

    // return the first index that it's element greater or equal than pdata.
    int indexOf(const void* pData) const;

    // insert a new attr into BLeafNode, return the index of the attr. return -1 if full.
    int leafInsert(void* attr, RID left);
    IX_BInsertUpEntry leafSpiltAndInsert(void* attr, RID rid, IX_BNodeWapper& newNode);

    // insert a up enrty(from child) into node, return the index of the attr, return -1 if full.
    int notLeafInsert(const IX_BInsertUpEntry& up);
    IX_BInsertUpEntry notLeafSpiltAndInsert(const IX_BInsertUpEntry& up, IX_BNodeWapper& newNode);

public:
    static void initNode(PF_PageHandle& page);
    static constexpr inline int countOrder(int attrLength)
    {
        // sizeof(BNode) = sizeof(int) + order * (attrLength) + (order + 1) * sizeof(rid) < PF_PAGE_SIZE
        return (PF_PAGE_SIZE - sizeof(int) - sizeof(RID)) / (attrLength + sizeof(RID));
    }

private:
    void spiltInto(IX_BNodeWapper& other, IX_BInsertUpEntry& up, bool isLeaf);
    // insert data to index i
    void insertInto(int i, RID rid);
    void insertInto(int i, char* pData);

private:
    // status of bNode
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
    char* attrs_;
};

#endif