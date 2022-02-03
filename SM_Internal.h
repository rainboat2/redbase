#ifndef SM_INTERNAL_HH
#define SM_INTERNAL_HH

#include "redbase.h"

#define MAXNAME 31
#define RELATION_TABLE_NAME "relcat"
#define ATTRIBUTE_TABLE_NAME "attrcat"

// Used by SM_Manager::CreateTable
struct AttrInfo {
    char attrName[MAXNAME + 1];
    AttrType attrType;
    int attrLength;
};

// Used by Printer class
struct DataAttrInfo {
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
};

// meta data, record all relations in database
struct Relcat {
    char relName[MAXNAME + 1];
    int tupleLength;
    int attrCount;
    int indexCount;
};

// meta data, record attribute information for each relation
struct Attrcat {
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
};

#endif
