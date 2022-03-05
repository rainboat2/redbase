#include "SM_NodeInterpter.h"

SM_NodeInterpter::SM_NodeInterpter(SM_Manager& sm, QL_Manager& ql)
    : sm_(sm)
    , ql_(ql)
{
}

RC SM_NodeInterpter::interp(Node* n)
{
    switch (n->kind) {
    case NodeKind::CREATE_TABLE:
        return createTable(n);
    case NodeKind::SELECT:
        return select(n);
    default:
        return RC::SM_UNKOWN_NODE;
    }
}

RC SM_NodeInterpter::createTable(Node* n)
{
    assert(n->kind == NodeKind::CREATE_TABLE);
    const char* relName = n->u.createTable.relName;
    int cnt = listLen(n->u.createTable.attrWithTypes);

    auto* attrs = new AttrInfo[cnt];
    Node* cur = n->u.createTable.attrWithTypes;
    for (int i = 0; i < cnt; i++, cur = cur->u.list.next) {
        Node* val = cur->u.list.val;

        attrs[i].attrLength = val->u.attrType.length;
        attrs[i].attrType = val->u.attrType.type;
        strncpy(attrs[i].attrName, val->u.attrType.attrName, MAXNAME + 1);
    }

    RC rc = sm_.CreateTable(relName, cnt, attrs);
    delete[] attrs;

    return rc;
}

RC SM_NodeInterpter::select(Node* n)
{
    assert(n->kind == NodeKind::SELECT);
    std::cout << n << std::endl;
    return RC::SUCCESS;
}

int SM_NodeInterpter::listLen(Node* n)
{
    assert(n->kind == NodeKind::LIST);
    int len = 0;
    Node* cur = n;
    while (cur != nullptr) {
        cur = cur->u.list.next;
        len++;
    }
    return len;
}