#ifndef RM_RID_HH
#define RM_RID_HH

#include "RM_Internal.h"
#include "pf.h"

class RID {
public:
    friend class RM_FileHandle;

public:
    RID();

    RID(PageNum pageNum, SlotNum slotNum);

    ~RID() = default;

    inline bool operator==(const RID& rid) const
    {
        return pageNum_ == rid.pageNum_ && slotNum_ == rid.slotNum_;
    }

    inline bool operator!=(const RID& rid) const
    {
        return pageNum_ != rid.pageNum_ || slotNum_ != rid.slotNum_;
    }

    RC GetPageNum(PageNum& pageNum) const;

    RC GetSlotNum(SlotNum& slotNum) const;

private:
    PageNum pageNum_;
    SlotNum slotNum_;
};

#define NULL_SLOT_NUM -1

const RID NULL_RID { NULL_PAGE_NUM, NULL_SLOT_NUM };

#define EXTRACT_SLOT_NUM(var, rid_object) \
    SlotNum var;                          \
    rid_object.GetSlotNum(var);

#define EXTRACT_PAGE_NUM(var, rid_object) \
    PageNum var;                          \
    rid_object.GetPageNum(var);

#define EXTRACT_PAGE_SLOT_NUM(pageNum, slotNum, rid_object) \
    EXTRACT_SLOT_NUM(slotNum, rid_object)                   \
    EXTRACT_PAGE_NUM(pageNum, rid_object)

#endif