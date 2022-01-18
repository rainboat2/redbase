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

bool RID::operator==(const RID &rid) const
{
    return pageNum_ == rid.pageNum_ && slotNum_ == rid.slotNum_;
}

RC RID::GetPageNum(PageNum& pageNum) const
{
    pageNum = pageNum_;
    return RC::SUCCESSS;
}

RC RID::GetSlotNum(SlotNum& slotNum) const
{
    slotNum = slotNum_;
    return RC::SUCCESSS;
}