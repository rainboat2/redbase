#include "ql.h"

QL_Manager::QL_Manager(SM_Manager& smm, IX_Manager& ixm, RM_Manager& rmm)
    : smm_(smm)
    , ixm_(ixm)
    , rmm_(rmm)
{
}
