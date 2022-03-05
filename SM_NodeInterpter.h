#ifndef SM_NODE_INTERPTER_HH
#define SM_NODE_INTERPTER_HH

#include "parser.h"
#include "ql.h"

class SM_NodeInterpter {
public:
    SM_NodeInterpter(SM_Manager& sm, QL_Manager& ql);

    RC interp(Node* n);

private:
    RC createTable(Node* n);
    RC select(Node* n);
    int listLen(Node* n);

private:
    SM_Manager& sm_;
    QL_Manager& ql_;
};

#endif