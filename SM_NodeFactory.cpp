#include "SM_NodeFactory.h"

#include <iostream>

SM_NodeFactory* SM_NodeFactory::getInstance()
{
    static SM_NodeFactory* factory = nullptr;
    if (factory == nullptr) {
        factory = new SM_NodeFactory();
    }
    return factory;
}

void SM_NodeFactory::resetMemory()
{
    nodeptr_ = 0;
}

Node* SM_NodeFactory::tableNode(char* relName, Node* attrWithTypes)
{
    Node* n = makeNode(NodeKind::CREATE_TABLE);
    n->u.createTable.relName = relName;
    n->u.createTable.attrWithTypes = attrWithTypes;
    return n;
}

Node* SM_NodeFactory::makeNode(NodeKind kind)
{
    if (nodeptr_ == MAX_NODE) {
        std::cerr << "node creator out of memory" << std::endl;
        exit(1);
    }
    Node* n = pool_ + nodeptr_;
    nodeptr_++;
    n->kind = kind;
    return n;
}

Node* SM_NodeFactory::relAttrNode(char* relName, char* attrName)
{
    Node* relAttr = makeNode(NodeKind::REL_ATTR);
    relAttr->u.relAttr.relName = relName;
    relAttr->u.relAttr.attrName = attrName;
    return relAttr;
}

Node* SM_NodeFactory::relNode(char* relName){
    Node* rel = makeNode(NodeKind::RELATION);
    rel->u.relation.relName = relName;
    return rel;
}

Node* SM_NodeFactory::listNode(Node* value)
{
    Node* listNode = makeNode(NodeKind::LIST);
    listNode->u.list.val = value;
    listNode->u.list.next = nullptr;
    return listNode;
}

Node* SM_NodeFactory::prepend(Node* list, Node* value)
{
    assert(list->kind == NodeKind::LIST);
    Node* newHead = listNode(value);
    newHead->u.list.next = list;
    return newHead;
}

Node* SM_NodeFactory::attrWithTypeNode(char* attrName, AttrType attrType, int attrLength)
{
    Node* n = makeNode(NodeKind::ATTR_WITH_TYPE);
    n->u.attrType.attrName = attrName;
    n->u.attrType.type = attrType;
    n->u.attrType.length = attrLength;
    return n;
}