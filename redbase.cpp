#include "parser.h"

#include <iostream>

int main(int argc, char* argv[]){
    const char* query = "create table person(age int);";
    Node* n = sqlParse(query);

    std::cout << n << std::endl;

    return 0;
}

