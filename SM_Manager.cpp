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
    return RC::SUCCESSS;
}