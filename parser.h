#ifndef PARSER_HH
#define PARSER_HH

#include "redbase.h"

enum class BoolOp {
    RD_AND,
    RD_OR,
    RD_NOT
};

struct RelAttr {
    char* relName; // Relation name (may be NULL)
    char* attrName; // Attribute name
};

struct AggRelAttr {
    // AggFun   func;
    char* relName; // Relation name (may be NULL)
    char* attrName; // Attribute name
};

struct Value {
    AttrType type; /* type of value               */
    void* data; /* value                       */
};

struct Condition {
    RelAttr lhsAttr; /* left-hand side attribute            */
    CompOp op; /* comparison operator                 */
    int bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
    /* in which case rhsAttr below is valid;*/
    /* otherwise, rhsValue below is valid.  */
    RelAttr rhsAttr; /* right-hand side attribute            */
    Value rhsValue; /* right-hand side value                */
};

enum class NodeKind {
    SELECT,
    CREATE_TABLE,
    LIST,
    RELATION,
    REL_ATTR,
    ATTR_VAL,
    ATTR_WITH_TYPE,
    VALUE,
    CONDITION,
    CONDITION_TREE,
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

        // leaf node of a condTree is condition node
        struct {
            BoolOp op;
            Node* left;
            Node* right;
        } condTree;

        struct {
            Node* relAttrList;
            Node* relList;
            Node* condTree;
        } select;

        struct {
            char* attrName;
            AttrType type;
            int length;
        } attrType;

        struct {
            char* relName; // may be null
            char* attrName;
        } relAttr;

        struct {
            char* relName;
        } relation;

        struct {
            Node* lRelAttr;
            CompOp op;
            Node* rRelAttrOrValue;
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