#ifndef RM_RID_HH
#define RM_RID_HH

class RID {
public:
    RID();
    
    ~RID();
    
    RID(PageNum pageNum, SlotNum slotNum);

    RC GetPageNum(PageNum& pageNum) const;

    RC GetSlotNum(SlotNum& slotNum) const;
};

#endif