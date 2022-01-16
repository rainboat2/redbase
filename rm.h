#ifndef RM_HH
#define RM_HH

#include "RM_RID.h"
#include "pf.h"
#include "redbase.h"

class RM_FileHandle;
class RM_FileScan;
class RM_Record;

class RM_Manager {
public:
    RM_Manager(PF_Manager& pf);
    ~RM_Manager() = default;
    RC CreateFile(const char* fileName, int recordSize);

    RC DestroyFile(const char* fileName);
    RC OpenFile(const char* fileName, RM_FileHandle& fileHandle);
    RC CloseFile(RM_FileHandle& fileHandle);

private:
    static int countRecordNums(int recordSize);

private:
    PF_Manager& pf_manager_;
};

class RM_FileHandle {
public:
    RM_FileHandle();
    ~RM_FileHandle();

    RC GetRec(const RID& rid, RM_Record& rec) const;
    RC InsertRec(const char* pData, RID& rid);
    RC DeleteRec(const RID& rid);
    RC UpdateRec(const RM_Record& rec);
    RC ForcePages(PageNum pageNum = ALL_PAGES) const;

    friend class RM_Manager;

private:
    RC getFreeSlot(RID& rid);
    RC writeDataToSlot(const char* pData, RID& rid);
    RC AllocateNewPage(PF_PageHandle& page);
    RC MarkPageAsFull(PF_PageHandle& page);
    RC MarkPageAsNotFull(PF_PageHandle& page);
    RC ForceHeader();

private:
    PF_FileHandle pf_fileHandle_;
    RM_FileHeader fileHeader_;
    bool isHeaderChange_;
    bool isOpen;
};

class RM_FileScan {
public:
    RM_FileScan();
    ~RM_FileScan();
    RC OpenScan(const RM_FileHandle& fileHandle,
        AttrType attrType,
        int attrLength,
        int attrOffset,
        CompOp compOp,
        void* value,
        ClientHint pinHint = ClientHint::NO_HINT);
    RC GetNextRec(RM_Record& rec);
    RC CloseScan();
};

class RM_Record {
public:
    RM_Record();
    ~RM_Record();

    // Set pData to point to the record's contents
    RC GetData(char*& pData) const;

    RC GetRid(RID& rid) const;

    friend class RM_FileHandle;

private:
    void setData(char* buf, int size);

private:
    char* data_;
    RID rid_;
};

#endif