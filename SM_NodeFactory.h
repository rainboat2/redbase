#ifndef SM_NODE_FACTORY_HH
#define SM_NODE_FACTORY_HH

#include "parser.h"

#define MAX_NODE 100

class SM_NodeFactory{
public:
    static SM_NodeFactory* getInstance();

public:
    void resetMemory();
    Node* tableNode(char* relName, Node* attrWithTypes);

    Node* listNode(Node* value);
    Node* relAttrNode(char* relName, char* attrName);
    Node* relNode(char* relName);
    Node* prepend(Node* list, Node* value);
    Node* attrWithTypeNode(char* attrName, AttrType attrType, int attrLength);

    Node* valueNode(int i);
    Node* valueNode(float f);
    Node* valueNode(char* str);

    Node* conditionNode(Node* relAttr, CompOp op, Node* valueOrRelAttr);
    Node* conditionTreeNode(Node* left, BoolOp op, Node* right);
    Node* selectNode(Node* relAttrList, Node* relList, Node* condTree);

private:
    SM_NodeFactory() = default;
    Node* makeNode(NodeKind kind);

private:
    Node pool_[MAX_NODE];
    int nodeptr_ = 0;
};

#endif