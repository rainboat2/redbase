#include "sm.h"

#include <string.h>

SM_Manager::SM_Manager(IX_Manager& ixm, RM_Manager& rmm)
    : ixm_(ixm)
    , rmm_(rmm)
{
    rmm_.OpenFile(RELATION_TABLE_NAME, relTable_);
    rmm_.OpenFile(ATTRIBUTE_TABLE_NAME, attrTable_);
}

SM_Manager::~SM_Manager()
{
    rmm_.CloseFile(relTable_);
    rmm_.CloseFile(attrTable_);
}

RC SM_Manager::CreateTable(const char* relName, int attrCount, AttrInfo* attributes)
{
    Relcat rel;
    for (int i = 0; i < attrCount; i++)
        rel.tupleLength += attributes[i].attrLength;
    rel.indexCount = 0;
    rel.attrCount = attrCount;
    strcpy(rel.relName, relName);
    RID addr;
    RETURN_RC_IF_NOT_SUCCESS(relTable_.InsertRec((char*)&rel, addr));

    Attrcat attr;
    strcpy(attr.relName, relName);
    attr.offset = 0;
    attr.indexNo = -1;
    for (int i = 0; i < attrCount; i++) {
        strcpy(attr.attrName, attributes[i].attrName);
        attr.attrLength = attributes[i].attrLength;
        attr.attrType = attributes[i].attrType;
        RETURN_RC_IF_NOT_SUCCESS(attrTable_.InsertRec((char*)&attr, addr));
        attr.offset += attributes[i].attrLength;
    }

    // create table file
    int recordSize = 0;
    for (int i = 0; i < attrCount; i++) {
        recordSize += attributes[i].attrLength;
    }
    RETURN_RC_IF_NOT_SUCCESS(rmm_.CreateFile(relName, recordSize));
    return RC::SUCCESS;
}

RC SM_Manager::DropTable(const char* relName)
{
    // read relation record
    RM_Record relRec;
    RETURN_RC_IF_NOT_SUCCESS(getRecord(relName, relRec));

    // read all attribute reocord
    std::vector<RM_Record> attrRecs;
    RETURN_RC_IF_NOT_SUCCESS(getAllAttrRecords(relName, attrRecs));

    // delete all index files
    for (auto& rec : attrRecs) {
        char* buf;
        rec.GetData(buf);
        Attrcat* attr = (Attrcat*)buf;
        if (attr->indexNo != -1)
            ixm_.DestroyIndex(attr->relName, attr->indexNo);
    }

    // delete table file
    RETURN_RC_IF_NOT_SUCCESS(rmm_.DestroyFile(relName));

    // delete attribute record;
    for (auto& rec : attrRecs) {
        RID addr;
        rec.GetRid(addr);
        attrTable_.DeleteRec(addr);
    }

    // delete relation record;
    {
        RID addr;
        relRec.GetRid(addr);
        relTable_.DeleteRec(addr);
    }
    return RC::SUCCESS;
}

RC SM_Manager::CreateIndex(const char* relName, const char* attrName)
{
    char* buf;

    // read relation record
    RM_Record relRec;
    RETURN_RC_IF_NOT_SUCCESS(getRecord(relName, relRec));
    relRec.GetData(buf);
    Relcat* rel = (Relcat*)buf;

    // read attribute record
    RM_Record attrRec;
    RETURN_RC_IF_NOT_SUCCESS(getRecord(relName, attrName, attrRec));
    attrRec.GetData(buf);
    Attrcat* attr = (Attrcat*)buf;

    // create index for attr
    if (attr->indexNo != -1)
        return RC::SM_INDEX_ARLEADY_EXIST;

    rel->indexCount++;
    attr->indexNo = rel->indexCount;
    RETURN_RC_IF_NOT_SUCCESS(
        ixm_.CreateIndex(
            rel->relName,
            attr->indexNo,
            attr->attrType,
            attr->attrLength));
    RETURN_RC_IF_NOT_SUCCESS(relTable_.UpdateRec(relRec));
    RETURN_RC_IF_NOT_SUCCESS(attrTable_.UpdateRec(attrRec));

    return RC::SUCCESS;
}

RC SM_Manager::DropIndex(const char* relName, const char* attrName)
{
    char* buf;
    // read relation record
    RM_Record relRec;
    RETURN_RC_IF_NOT_SUCCESS(getRecord(relName, relRec));
    relRec.GetData(buf);
    const Relcat* rel = (Relcat*)buf;

    // read attribute record
    RM_Record attrRec;
    RETURN_RC_IF_NOT_SUCCESS(getRecord(relName, attrName, attrRec));
    attrRec.GetData(buf);
    Attrcat* attr = (Attrcat*)buf;

    if (attr->indexNo == 1)
        return RC::SUCCESS;

    RETURN_RC_IF_NOT_SUCCESS(ixm_.DestroyIndex(rel->relName, attr->indexNo));
    attr->indexNo = -1;
    RETURN_RC_IF_NOT_SUCCESS(attrTable_.UpdateRec(attrRec));
    return RC::SUCCESS;
}

RC SM_Manager::getRecord(const char* relName, RM_Record& relRec)
{
    RM_FileScan scan;
    RC rc = scan.OpenScan(relTable_,
        AttrType::RD_STRING,
        MAXNAME,
        0,
        CompOp::EQ,
        const_cast<char*>(relName));
    RETURN_RC_IF_NOT_SUCCESS(rc);

    RETURN_RC_IF_NOT_SUCCESS(scan.GetNextRec(relRec));
    RETURN_RC_IF_NOT_SUCCESS(scan.CloseScan());
    return RC::SUCCESS;
}

RC SM_Manager::getRecord(const char* relName, const char* attrName, RM_Record& attrRec)
{
    RM_FileScan scan;
    RC rc = scan.OpenScan(relTable_,
        AttrType::RD_STRING,
        MAXNAME,
        0,
        CompOp::EQ,
        const_cast<char*>(relName));

    Attrcat* attr = nullptr;
    while ((rc = scan.GetNextRec(attrRec)) == RC::SUCCESS) {
        char* buf;
        attrRec.GetData(buf);
        attr = (Attrcat*)buf;
        if (strcmp(attrName, attr->attrName) == 0) {
            break;
        } else {
            attr = nullptr;
        }
    }
    RETURN_RC_IF_NOT_SUCCESS(scan.CloseScan());

    if (rc != RC::SUCCESS && rc != RC::RM_FILE_EOF)
        return rc;
    if (attr == nullptr)
        return RC::SM_ATTR_NOT_FOUND;
    return RC::SUCCESS;
}

RC SM_Manager::getAllAttrRecords(const char* relName, std::vector<RM_Record>& attrs)
{
    RM_FileScan scan;
    RC rc = scan.OpenScan(relTable_,
        AttrType::RD_STRING,
        MAXNAME,
        0,
        CompOp::EQ,
        const_cast<char*>(relName));

    RM_Record attrRec;
    while ((rc = scan.GetNextRec(attrRec)) == RC::SUCCESS) {
        char* buf;
        attrRec.GetData(buf);
        attrs.push_back(std::move(attrRec));
    }
    RETURN_RC_IF_NOT_SUCCESS(scan.CloseScan());

    if (rc != RC::SUCCESS && rc != RC::RM_FILE_EOF)
        return rc;
    if (attrs.size() == 0)
        return RC::SM_ATTR_NOT_FOUND;

    return RC::SUCCESS;
}