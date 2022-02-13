#include "ix.h"
#include "pf.h"
#include "redbase.h"
#include "rm.h"
#include "sm.h"

#include <iostream>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " dbname \n";
        exit(1);
    }

    // The database name is the second argument
    const char* dbname = argv[1];
    if (mkdir(dbname, 0755)) {
        std::cerr << strerror(errno) << std::endl;
        exit(1);
    }

    if (chdir(dbname)) {
        std::cerr << strerror(errno) << std::endl;
        exit(1);
    }

    PF_Manager pf_manager;
    RM_Manager rm_manager(pf_manager);
    IX_Manager ix_manager(pf_manager);

    EXIT_IF_NOT_SUCESSS(rm_manager.CreateFile(RELATION_TABLE_NAME, sizeof(Relcat)));
    EXIT_IF_NOT_SUCESSS(rm_manager.CreateFile(ATTRIBUTE_TABLE_NAME, sizeof(Attrcat)));

    SM_Manager sm_manager(ix_manager, rm_manager);
    EXIT_IF_NOT_SUCESSS(sm_manager.OpenDb(dbname));

    AttrInfo relcatAttrs[] = {
        AttrInfo { "relName", AttrType::RD_STRING, MAXNAME + 1 },
        AttrInfo { "tupleLength", AttrType::RD_INT, sizeof(int) },
        AttrInfo { "attrCount", AttrType::RD_INT, sizeof(int) },
        AttrInfo { "indexCount", AttrType::RD_INT, sizeof(int) },
    };
    EXIT_IF_NOT_SUCESSS(sm_manager.CreateTable(RELATION_TABLE_NAME, sizeof(relcatAttrs) / sizeof(AttrInfo), relcatAttrs));

    AttrInfo attrcatAttrs[] = {
        AttrInfo { "relName", AttrType::RD_STRING, MAXNAME + 1 },
        AttrInfo { "attrName", AttrType::RD_STRING, MAXNAME + 1 },
        AttrInfo { "offset", AttrType::RD_INT, sizeof(int) },
        AttrInfo { "attrType", AttrType::RD_INT, sizeof(int) },
        AttrInfo { "attrLength", AttrType::RD_INT, sizeof(int) },
        AttrInfo { "indexNo", AttrType::RD_INT, sizeof(int) }
    };
    EXIT_IF_NOT_SUCESSS(sm_manager.CreateTable(ATTRIBUTE_TABLE_NAME, sizeof(attrcatAttrs) / sizeof(AttrInfo), attrcatAttrs));

    EXIT_IF_NOT_SUCESSS(sm_manager.CloseDb());
    return 0;
}