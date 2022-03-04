#ifndef PARSER_HH
#define PARSER_HH

#include "redbase.h"

struct RelAttr {
    char* relName; // Relation name (may be NULL)
    char* attrName; // Attribute name
};

struct AggRelAttr {
    // AggFun   func;
    char* relName; // Relation name (may be NULL)
    char* attrName; // Attribute name

    // Print function
};

struct Value {
    AttrType type; /* type of value               */
    void* data; /* value                       */
    /* print function              */
};

struct Condition {
    RelAttr lhsAttr; /* left-hand side attribute            */
    CompOp op; /* comparison operator                 */
    int bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
    /* in which case rhsAttr below is valid;*/
    /* otherwise, rhsValue below is valid.  */
    RelAttr rhsAttr; /* right-hand side attribute            */
    Value rhsValue; /* right-hand side value                */
    /* print function                               */
};

enum class NodeKind {
    CREATE_TABLE,
    LIST,
    RELATION,
    REL_ATTR,
    ATTR_VAL,
    ATTR_WITH_TYPE,
};

struct Node {
    NodeKind kind;
    union {
        struct {
            char* relName;
            Node* attrWithTypes;
        } createTable;

        struct
        {
            Node* relAttrList;
            Node* relList;
            Node* conditionList;
        } query;

        struct {
            Node* val;
            Node* next;
        } list;

        struct {
            char* attrName;
            AttrType type;
            int length;
        } attrType;

        struct {
            char* relName; // may be null
            char* attrName;
        } relAttr;

        struct{
            char* relName;
        } relation;

        struct {
            Node* lRelAttr;
            CompOp op;
            Node* rRelAttr;
            Node* rValue;
        } condition;

        struct {
            AttrType type;
            int ival;
            float fval;
            char* str;
        } value;
    } u;
};

Node* sqlParse(const char* query);

#endif