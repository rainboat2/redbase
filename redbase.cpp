#include "parser.h"

#include <iostream>

int main(int argc, char* argv[]){
    extern int yydebug;
    yydebug = 0;

    const char* q1 = "create table person(age int);";
    Node* n = sqlParse(q1);

    std::cout << n << std::endl;

    const char* q2 = "create table person(age int, name char(30));";
    n = sqlParse(q2);
    std::cout << n << std::endl;

    return 0;
}

