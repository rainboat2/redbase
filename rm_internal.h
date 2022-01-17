#ifndef RM_INTERNAL_HH
#define RM_INTERNAL_HH

#include <string.h>

using SlotNum = int;

struct RM_FileHeader {
    int pageNums;
    // point to the next free page
    int nextFree;
    int recordSize;
    int recordNumsOfEachPage;
};

#define BITMAP_SIZE(recordNums) \
    (recordNums / 8 + ((recordNums % 8 == 0) ? 0 : 1))

// page  structure
// (RM_PageHeader, bitmap, slot ... slot, free space)
// nextFree could be PageStatus or positive number(stand for next free page)
// size of bitmap is depend on RM_FileHeader.recordNumsOfEachPage, use marco BITMAP_SIZE can compute it
// bits in bitmap: 1 starnd for slot is used, 0 stand for slot is free
struct RM_PageHeader {
    int nextFree;
    char bitmap;

    static inline int size(int recordNums)
    {
        return sizeof(int) + BITMAP_SIZE(recordNums);
    }
};

class BitMapWapper {
public:
    BitMapWapper(char* data, size_t bitsNum);
    ~BitMapWapper() = default;

    inline size_t size() const { return bitsNum_; };
    void set(size_t i, bool b);
    bool get(size_t i) const;
    // true if all bits is set to 1, else return false
    bool all() const;
    int findFirstZero() const;

private:
    // record bits from left to right
    unsigned char* bitmap_;
    size_t bitsNum_;
    size_t bitmap_size_;
};

#endif