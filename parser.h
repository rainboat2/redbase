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

struct RelAttr{
    char     *relName;    // Relation name (may be NULL)
    char     *attrName;   // Attribute name

};

struct AggRelAttr{
    // AggFun   func; 
    char     *relName;    // Relation name (may be NULL)
    char     *attrName;   // Attribute name

    // Print function
};

struct Value{
    AttrType type;         /* type of value               */
    void     *data;        /* value                       */
			   /* print function              */
};

struct Condition{
    RelAttr  lhsAttr;    /* left-hand side attribute            */
    CompOp   op;         /* comparison operator                 */
    int      bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
                         /* in which case rhsAttr below is valid;*/
                         /* otherwise, rhsValue below is valid.  */
    RelAttr  rhsAttr;    /* right-hand side attribute            */
    Value    rhsValue;   /* right-hand side value                */
			 /* print function                               */

};

struct Node {
    NodeKind kind;
    union {
        struct {
            char* relName;
            Node* attrTypes;
        } createTable;

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