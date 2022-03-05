#include "redbase.h"
#include "SM_NodeInterpter.h"
#include "ix.h"
#include "parser.h"
#include "pf.h"
#include "ql.h"
#include "rm.h"
#include "sm.h"

#include <iostream>
#include <readline/readline.h>

int main(int argc, char* argv[])
{
    if (argc < 2){
        std::cerr << "Usage: \n\tredbase databasePath" << std::endl;
        exit(1);
    }
    extern int yydebug;
    yydebug = 1;
    PF_Manager pf;
    RM_Manager rm(pf);
    IX_Manager ix(pf);
    SM_Manager sm(ix, rm);
    QL_Manager ql(sm, ix, rm);

    SM_NodeInterpter interp(sm, ql);

    const char* databasePath = argv[1];
    // open database
    EXIT_IF_NOT_SUCESSS(sm.OpenDb(databasePath));

    while (true) {
        const char* command = readline("redbase$ ");
        if (strcmp(command, "exit") == 0)
            break;

        Node* n = sqlParse(command);
        if (n == nullptr) continue;

        RC rc = interp.interp(n);
        if (rc != RC::SUCCESS){
            PrintError(rc);
        }
    }
    EXIT_IF_NOT_SUCESSS(sm.CloseDb());

    return 0;
}
