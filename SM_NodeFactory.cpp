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

Node* SM_NodeFactory::tableNode(char* relName, Node* attrTypes)
{
    Node* n = makeNode(NodeKind::CREATE_TABLE);
    n->u.createTable.relName = relName;
    n->u.createTable.attrTypes = attrTypes;
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

Node* SM_NodeFactory::listNode(Node* value){
    Node* listNode = makeNode(NodeKind::LIST);
    listNode->u.list.val = value;
    listNode->u.list.next = nullptr;
    return listNode;
}

Node* SM_NodeFactory::prepend(Node* list, Node* value){
    assert(list->kind == NodeKind::LIST);
    Node* newHead = listNode(value);
    newHead->u.list.next = list;
    return newHead;
}

Node* SM_NodeFactory::attrWithTypeNode(char* attrName, AttrType attrType, int attrLength){
    Node* n = makeNode(NodeKind::ATTR_WITH_TYPE);
    n->u.attrType.attrName = attrName;
    n->u.attrType.type = attrType;
    n->u.attrType.length = attrLength;
    return n;
}