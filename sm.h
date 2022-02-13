#ifndef SM_HH
#define SM_HH

#include "SM_Internal.h"
#include "ix.h"
#include "pf.h"
#include "redbase.h"
#include "rm.h"

#include <vector>

class SM_Manager {
public:
    SM_Manager(IX_Manager& ixm, RM_Manager& rmm);
    ~SM_Manager();

    RC OpenDb(const char* dbName);
    RC CloseDb();

    RC CreateTable(const char* relName, int attrCount, AttrInfo* attributes);
    RC DropTable(const char* relName);
    RC CreateIndex(const char* relName, const char* attrName);
    RC DropIndex(const char* relName, const char* attrName);

    RC Load(const char* relName, const char* fileName);
    RC Help();
    RC Help(const char* relName);
    RC Print(const char* relName);
    RC Set(const char* paramName, const char* value);

private:
    RC getRecord(const char* relName, RM_Record& relRec);
    RC getRecord(const char* relName, const char* attrName, RM_Record& attrRec);
    RC getAllAttrRecords(const char* relName, std::vector<RM_Record>& attrs);
    RC buildIndex(Attrcat& attr);

private:
    IX_Manager& ixm_;
    RM_Manager& rmm_;

    bool isOpen;
    RM_FileHandle relTable_;
    RM_FileHandle attrTable_;
};

#endif