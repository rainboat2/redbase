#ifndef PARSER_HH
#define PARSER_HH

#include "redbase.h"

enum class NodeKind {
    CREATE_TABLE,
    LIST,
    REL_ATTR,
    ATTR_VAL,
    ATTR_WITH_TYPE,
};

struct Node {
    NodeKind kind;
    union {
        struct {
            char* relName;
            Node* attrList;
        } createTable;

        struct {
            Node* val;
            Node* next;
        } list;

        struct {
            char* attrName;
            AttrType type;
            int length;
        } attrWithType;

        struct {
            char* relName; // may be null
            char* attrName;
        } relAttr;

        struct {
            char* attrName;
            AttrType valType;
            int varLength;
            void* value;
        } attrVal;
    } u;
};

Node* sqlParse(const char* query);

#endif