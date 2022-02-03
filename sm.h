#ifndef SM_HH
#define SM_HH

#include "ix.h"
#include "pf.h"
#include "redbase.h"
#include "rm.h"

#include "SM_Internal.h"

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
    IX_Manager& ixm_;
    RM_Manager& rmm_;
    RM_FileHandle relTable_;
    RM_FileHandle attrTable_;
};

#endif