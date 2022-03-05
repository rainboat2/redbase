#include "ql.h"

QL_Manager::QL_Manager(SM_Manager& smm, IX_Manager& ixm, RM_Manager& rmm)
    : smm_(smm)
    , ixm_(ixm)
    , rmm_(rmm)
{
}

RC QL_Manager::Select(
    int nSelAttrs,
    const RelAttr selAttrs[],
    int nRelations,
    const char* const relations[],
    int nConditions,
    const Condition conditions[])
{
    return RC::SUCCESS;
}