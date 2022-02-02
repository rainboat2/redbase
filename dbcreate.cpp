#include "ix.h"
#include "pf.h"
#include "redbase.h"
#include "rm.h"

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

    if (chdir(dbname)){
        std::cerr << strerror(errno) << std::endl;
        exit(1);
    }

    // Create the system catalogs
    std::cout << "create system catalogs" << std::endl;
    return 0;
}