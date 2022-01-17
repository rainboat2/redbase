#ifndef RM_RID_HH
#define RM_RID_HH

#include "pf.h"
#include "RM_Internal.h"

class RID {
public:
    RID();
    
    ~RID() = default;
    
    RID(PageNum pageNum, SlotNum slotNum);

    RC GetPageNum(PageNum& pageNum) const;

    RC GetSlotNum(SlotNum& slotNum) const;

    friend class RM_FileHandle;

private:
    PageNum pageNum_;
    SlotNum slotNum_;
};

#endif