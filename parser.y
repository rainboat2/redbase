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

void reset_scanner(const char* query);
void reset_parser();
%}

%union{
    char* str;
    int ival;
    float fval;
    AttrType aType;
    CompOp compOp;
    Node* node;
}

%token<ival>  CREATE TABLE INT_T CHAR_T FLOAT_T

%token <str> STRING
%token <ival> INT
%token <fval> FLOAT


%type <node> command ddl createStmt attr_with_type_list attr_with_type
%type <str>  utility attrName
%type <aType> attrType

%%

start:
    command ';'{
        parser_tree = $1;
        YYACCEPT;
    }
    ;

command:
    ddl
    utility
    ;

utility:
    STRING
    ;

ddl:
    createStmt
    ;

createStmt:
    CREATE TABLE STRING '(' attr_with_type_list ')'
    {
        $$ = nodeFactory->tableNode($3, $5);
    }
    ;

attr_with_type_list:
    attr_with_type ',' attr_with_type_list{
        $$ = nodeFactory->prepend($1, $3);
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
    attrName attrType '(' INT_T ')'{
        $$ = nodeFactory->attrWithTypeNode($1, $2, $4);
    }
    ;

attrName:
    STRING
    ;

attrType:
    STRING{
        if (strncmp($1, "int", 4) == 0){
            $$ = AttrType::RD_INT;
        }else if (strncmp($1, "float", 6) == 0){
            $$ = AttrType::RD_FLOAT;
        }else if (strncmp($1, "char", 5) == 0){
            $$ = AttrType::RD_STRING;
        }else{
            yyerror("unkown type!");
        }
    }
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
   puts(s);
}

void reset_parser(){
    nodeFactory->resetMemory();
    parser_tree = nullptr;
}