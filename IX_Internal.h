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
// all element in rid_i less than attr_i
// all element in rid_(i+1) that not appear in before node equal or greater then attr_i
class IX_BNodeWapper {
public:
    IX_BNodeWapper();
    IX_BNodeWapper(int attrLength, AttrType attrType_, char* nodeData, RID addr);
    ~IX_BNodeWapper() = default;

    inline int size() const { return *size_; };
    inline bool isFull() const { return *size_ == order_; }

    inline void setRid(int i, RID rid) { rids_[i] = rid; }
    inline RID getRid(int i) const { return rids_[i]; }
    inline RID lastRid() const {return rids_[order_];}
    inline void setAttr(int i, void* data) { memcpy(getAttr(i), data, attrLength_); }
    inline void* getAttr(int i) const { return attrs_ + attrLength_ * i; }

    // get the pageNum where this node locate
    inline PageNum getPageNum()
    {
        PageNum num;
        addr_.GetPageNum(num);
        return num;
    }

    // return the first index that it's element greater then pData
    int upperBound(const void* pData) const;

    // return the last index that it's element less then pData
    int lowerBound(const void* pdata) const;

    // return the first index that it's element equal to pData, return -1 if not found
    int indexOf(const void* pData) const;

    // insert a new attr into BLeafNode, return the index of the attr. return -1 if full.
    int leafInsert(void* attr, RID left);
    IX_BInsertUpEntry leafSpiltAndInsert(void* attr, RID rid, IX_BNodeWapper& newNode);

    // insert a up enrty(from child) into node, return the index of the attr, return -1 if full.
    int notLeafInsert(const IX_BInsertUpEntry& up);
    IX_BInsertUpEntry notLeafSpiltAndInsert(const IX_BInsertUpEntry& up, IX_BNodeWapper& newNode);

public:
    static void initNode(PF_PageHandle& page, AttrType attrType_, int attrLength_);
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