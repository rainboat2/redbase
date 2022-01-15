#ifndef RM_RID_HH
#define RM_RID_HH

#include "RM_Internal.h"
#include "pf.h"

class RID {
public:
    RID();

    ~RID() = default;

    RID(PageNum pageNum, SlotNum slotNum);

    RC GetPageNum(PageNum& pageNum) const;

    RC GetSlotNum(SlotNum& slotNum) const;

private:
    PageNum pageNum_;
    SlotNum slotNum_;
};

#endif