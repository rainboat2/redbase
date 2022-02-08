#ifndef IX_HH
#define IX_HH

#include "IX_Internal.h"
#include "pf.h"
#include "redbase.h"
#include "rm.h"

class IX_IndexHandle;
class IX_ManagerTest;
class IX_IndexScan;

class IX_Manager {
public:
    IX_Manager(PF_Manager& pfm);
    ~IX_Manager() = default;
    RC CreateIndex(const char* fileName,
        int indexNo,
        AttrType attrType,
        int attrLength);
    RC DestroyIndex(const char* fileName,
        int indexNo);
    RC OpenIndex(const char* fileName,
        int indexNo,
        IX_IndexHandle& indexHandle);
    RC CloseIndex(IX_IndexHandle& indexHandle);
    std::string getFileName(const char* filename, int indexNo);

private:
    void setIndexHeader(PF_PageHandle& headerPage, AttrType attrtype, int attrLength);

private:
    PF_Manager& pf_manager_;
};

class IX_BBucketIterator;

class IX_IndexHandle {
    friend class IX_Manager;
    friend class IX_IndexScan;
    friend class IX_BBucketIterator;

public:
    IX_IndexHandle();
    ~IX_IndexHandle();
    IX_IndexHandle(const IX_IndexHandle& handle) = delete;
    IX_IndexHandle& operator=(const IX_IndexHandle& handle) = delete;

    RC InsertEntry(void* pData, const RID& rid);
    RC DeleteEntry(void* pData, const RID& rid);

    // find entry in leaf node that pData first appear, -1 will be set to slotNum if no entry can be found
    RC GetLeafEntryAddrEqualTo(void* pData, RID& rid) const;

    // find leaf node that element greater than pData first appear
    RC GetLeafEntryAddrGreatThen(void* pData, RID& rid) const;
    // find first leaf node
    RC GetFirstLeafEntryAddr(RID& rid) const;
    RC ForcePages();

private:
    void insertIntoBucket(IX_BNodeWapper& leaf, void* pData, const RID& rid);
    // in order to distinguish between nodeAddr and bucketAddr, pageNum of bucketAddr is negative number
    static bool isBucketAddr(const RID& rid);
    // buckets is orgainze as a single linked list, this function find a not full bucket in linked list
    RID findNotFullBucket(const RID& bucketAddr, IX_BNodeWapper& leaf, int);
    RID findNewBucketFrom(IX_BNodeWapper& leaf, int i);
    RID appendBucketIfFull(IX_BNodeWapper& leaf, RID tailAddr, int i);

    IX_BInsertUpEntry InsertEntry(IX_BNodeWapper& cur, void* pData, const RID& rid, int level);
    IX_BDeleteUpEntry DeleteEntry(IX_BNodeWapper& cur, void* pData, const RID& rid, int level);
    void changeRoot(IX_BInsertUpEntry& entry);

    RC getLeafBy(void* pData, RID& rid, std::function<RID(void*, IX_BNodeWapper&)> getNext) const;

    IX_BBucketIterator getBucketIterator(const RID& rid) const;

    IX_BNodeWapper readBNodeFrom(const RID& rid) const;
    IX_BBucketListWapper readBucketListFrom(const RID& rid) const;

    IX_BNodeWapper createBNode();
    IX_BBucketListWapper createBucketList();

    RC forceHeader();
    inline RC markDirty(IX_BNodeWapper& node) const { return pf_fileHandle_.MarkDirty(node.getPageNum()); }
    inline RC markDirty(IX_BBucketListWapper& bucketList) const { return pf_fileHandle_.MarkDirty(-bucketList.getPageNum()); }
    inline RC unpin(IX_BNodeWapper& node) const { return pf_fileHandle_.UnpinPage(node.getPageNum()); }
    inline RC unpin(IX_BBucketListWapper& bucketList) const { return pf_fileHandle_.UnpinPage(-bucketList.getPageNum()); }

private:
    IX_BNodeWapper root_;
    IX_BFileHeader fileHeader_;
    PF_FileHandle pf_fileHandle_;
    bool isOpen_;
    bool isHeaderChange_;
};

class IX_BBucketIterator {
    friend class IX_IndexHandle;

private:
    IX_BBucketIterator(const IX_IndexHandle* indexHandle, RID bucketAddr);

public:
    IX_BBucketIterator();
    ~IX_BBucketIterator();
    bool hasNext() const;
    RID next();

private:
    const IX_IndexHandle* indexHandle_;
    IX_BBucketListWapper bucketList_;
    IX_BBucketWapper bucket_;
    int pos_;
};

class IX_IndexScan {
public:
    IX_IndexScan();
    ~IX_IndexScan();
    RC OpenScan(const IX_IndexHandle& indexHandle,
        CompOp compOp,
        void* value,
        ClientHint pinHint = ClientHint::NO_HINT);
    RC GetNextEntry(RID& rid);
    RC CloseScan();

private:
    RC findFirstNode();
    inline RID curRid()
    {
        EXTRACT_SLOT_NUM(slot, curPos_);
        return curNode_.getRid(slot);
    }

    void moveToNextValidEntry();
    bool moveToNextEntry();

private:
    IX_IndexHandle* indexHandle_;
    std::function<int(const void*, const void*)> cmp_;
    CompOp compOp_;
    void* value_;
    ClientHint pinHint_;

    IX_BNodeWapper curNode_;
    IX_BBucketIterator bucketIt_;
    RID curPos_;
    bool isOpen_;
    bool hasNextBucket_;
};

#endif