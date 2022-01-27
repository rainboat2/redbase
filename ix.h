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
    void setIndexHeader(PF_PageHandle& hdaderPage, AttrType attrtype, int attrLength);

private:
    PF_Manager& pf_manager_;
};

class IX_IndexHandle {
    friend class IX_Manager;
    friend class IX_IndexScan;

public:
    IX_IndexHandle();
    ~IX_IndexHandle();
    IX_IndexHandle(const IX_IndexHandle& handle) = delete;
    IX_IndexHandle& operator=(const IX_IndexHandle& handle) = delete;

    RC InsertEntry(void* pData, const RID& rid);
    RC DeleteEntry(void* pData, const RID& rid);
    RC ForcePages();

private:
    IX_BInsertUpEntry InsertEntry(IX_BNodeWapper& cur, void* pData, const RID& rid, int level);
    void changeRoot(IX_BInsertUpEntry& entry);
    IX_BNodeWapper readBNodeFrom(const RID& rid);
    IX_BNodeWapper createBNode();
    RC forceHeader();

private:
    IX_BNodeWapper root_;
    IX_BFileHeader fileHeader_;
    PF_FileHandle pf_fileHandle_;
    bool isOpen_;
    bool isHeaderChange_;
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
    bool isOpen_;
    IX_IndexHandle *indexHandle_;
    std::function<int(const void* d1, const void* d2)> cmp_;
    void* value_;
    ClientHint pinHint_;
};

#endif