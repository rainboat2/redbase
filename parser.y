%locations

%{
#include <string.h>
#include <iostream>

#include "redbase.h"
#include "parser.h"
#include "SM_NodeFactory.h"

auto* nodeFactory = SM_NodeFactory::getInstance();
static Node *parser_tree = nullptr;
int getTypeLen(AttrType attrType);

#ifndef yyerror
void yyerror(char const *s);
#endif

#ifndef yylex
int yylex();
#endif

#define YYDEBUG 1

void reset_scanner(const char* query);
void reset_parser();
%}

%union{
    char* str;
    int ival;
    float fval;
    AttrType attrType;
    CompOp compOp;
    Node* node;
}

%token  CREATE TABLE SELECT FROM WHERE INT_T CHAR_T FLOAT_T

%token <str> STRING
%token <ival> INT
%token <fval> FLOAT


%type <node> command ddl dml createStmt selectStmt attr_with_type_list attr_with_type where_clause_opt relAttrList
             relList relAttr

%type <str> attrName relName
%type <attrType> attrType

%%

start:
    command ';'{
        parser_tree = $1;
        YYACCEPT;
    }
    ;

command:
    ddl
    | dml
    ;


ddl:
    createStmt
    ;

dml:
    selectStmt
    ;


createStmt:
    CREATE TABLE STRING '(' attr_with_type_list ')'
    {
        $$ = nodeFactory->tableNode($3, $5);
    }
    ;

attr_with_type_list:
    attr_with_type ',' attr_with_type_list{
        $$ = nodeFactory->prepend($3, $1);
    }
    |
    attr_with_type
    {
        $$ = nodeFactory->listNode($1);
    }
    ;

attr_with_type:
    attrName attrType{
        $$ = nodeFactory->attrWithTypeNode($1, $2, getTypeLen($2));
    }
    |
    attrName attrType '(' INT ')'{
        $$ = nodeFactory->attrWithTypeNode($1, $2, $4);
    }
    ;

attrType:
    INT_T {
        $$ = AttrType::RD_INT;
    }
    |
    FLOAT_T {
        $$ = AttrType::RD_FLOAT;
    }
    |
    CHAR_T {
        $$ = AttrType::RD_STRING;
    }
    ;

selectStmt:
    SELECT relAttrList FROM relList where_clause_opt{

    }
    ;

where_clause_opt:
    WHERE conditionList{

    }
    |
    /* null */
    {
        $$ = nullptr;
    }
    ;

relAttrList:
    relAttr ',' relAttrList{
        $$ = nodeFactory->prepend($3, $1);
    }
    |
    relAttr{
        $$ = nodeFactory->listNode($1);
    }
    ;

relList:
    relName ',' relList{
        Node* rel = nodeFactory->relNode($1);
        $$ = nodeFactory->prepend($3, rel);
    }
    |
    relName{
        Node* rel = nodeFactory->relNode($1);
        $$ = nodeFactory->listNode(rel);
    }
    ;

relAttr:
    relName '.' attrName{
        $$ = nodeFactory->relAttrNode($1, $3);
    }
    |
    attrName{
        $$ = nodeFactory->relAttrNode(nullptr, $1);
    }
    ;

conditionList:
    {
    }
    ;

attrName:
    STRING
    ;

relName:
    STRING
    ;

%%

Node* sqlParse(const char* query){
    reset_parser();
    reset_scanner(query);
    if (yyparse() == 0 && parser_tree != nullptr){
        return parser_tree;
    }else{
        std::cerr << "parser error!" << std::endl;
        return nullptr;
    }
}

int getTypeLen(AttrType attrType){
    switch (attrType){
    case AttrType::RD_INT:
        return sizeof(int);    
    case AttrType::RD_FLOAT:
        return sizeof(float);
    case AttrType::RD_STRING:
        yyerror("length of type string should be specified by user");
    default:
        yyerror("unkown type");
    }
    return -1;
} 

void yyerror(char const *s)
{
   std::cout << yylloc.first_line << '.' << yylloc.first_column << '-'
   << yylloc.last_line << '.' << yylloc.last_column << ": error: " << s << std::endl;
}

void reset_parser(){
    nodeFactory->resetMemory();
    parser_tree = nullptr;
}