#ifndef QL_HH
#define QL_HH

#include "ix.h"
#include "pf.h"
#include "redbase.h"
#include "rm.h"
#include "sm.h"

#include "parser.h"

class QL_Manager {
public:
    QL_Manager(SM_Manager& smm, IX_Manager& ixm, RM_Manager& rmm);
    ~QL_Manager();

    RC Select(int nSelAttrs,
        const RelAttr selAttrs[],
        int nRelations,
        const char* const relations[],
        int nConditions,
        const Condition conditions[]);

    RC Insert(const char* relName,
        int nValues,
        const Value values[]);

    RC Delete(const char* relName,
        int nConditions,
        const Condition conditions[]);

    RC Update(const char* relName,
        const RelAttr& updAttr,
        // 0/1 if RHS of = is attribute/value
        const int bIsValue,
        // attr on RHS of =
        const RelAttr& rhsRelAttr,
        // value on RHS of =
        const Value& rhsValue,
        int nConditions,
        const Condition conditions[]);
};

#endif