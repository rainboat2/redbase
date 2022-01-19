#ifndef IX_HH
#define IX_HH

#include <memory>

#include "pf.h"
#include "redbase.h"
#include "rm.h"

class IX_IndexHandle;

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

private:
    std::unique_ptr<char[]> getFileName(const char* filename, int indexNo);
    void setIndexHeader(PF_PageHandle& hdaderPage, AttrType attrtype, int attrLength);

private:
    PF_Manager& pf_manager_;
};

class IX_IndexHandle {
public:
    IX_IndexHandle();
    ~IX_IndexHandle();
    RC InsertEntry(void* pData, const RID& rid);
    RC DeleteEntry(void* pData, const RID& rid);
    RC ForcePages();
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
};

#endif