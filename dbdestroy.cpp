#include "ix.h"
#include "pf.h"
#include "redbase.h"
#include "rm.h"
#include "sm.h"

#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define EXIT_IF_FAIL_UNIX_CALL(call)                   \
    {                                                  \
        if (call) {                                    \
            std::cerr << strerror(errno) << std::endl; \
            exit(-1);                                  \
        }                                              \
    }

// dfs delete directory
int remove_dir(const char* path)
{
    if (access(path, F_OK)) {
        return 0;
    }

    struct stat dir_stat;
    EXIT_IF_FAIL_UNIX_CALL(stat(path, &dir_stat));

    if (S_ISREG(dir_stat.st_mode)) {
        EXIT_IF_FAIL_UNIX_CALL(unlink(path));
    } else if (S_ISDIR(dir_stat.st_mode)) {
        DIR* dir = opendir(path);
        struct dirent *dp;
        while ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;
            char dir_path[255];
            sprintf(dir_path, "%s/%s", path, dp->d_name);
            remove_dir(dir_path);
        }
        closedir(dir);
        EXIT_IF_FAIL_UNIX_CALL(rmdir(path));
    }else{
        std::cerr << "unexpected file type!" << std::endl;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " dbname \n";
        exit(1);
    }
    remove_dir(argv[1]);
    return 0;
}