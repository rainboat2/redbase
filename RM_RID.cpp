#include "RM_RID.h"
#include "redbase.h"

RID::RID()
    : pageNum_(-1)
    , slotNum_(-1)
{
}

RID::RID(PageNum pageNum, SlotNum slotNum)
    : pageNum_(pageNum)
    , slotNum_(slotNum)
{
}

RC RID::GetPageNum(PageNum& pageNum) const
{
    pageNum = pageNum_;
    return RC::SUCCESS;
}

RC RID::GetSlotNum(SlotNum& slotNum) const
{
    slotNum = slotNum_;
    return RC::SUCCESS;
}