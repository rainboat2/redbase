#ifndef RM_INTERNAL_HH
#define RM_INTERNAL_HH

#include "BitMapWapper.h"

using SlotNum = int;

struct RM_FileHeader {
    int pageNums;
    // point to the next free page
    int nextFree;
    int recordSize;
    int recordNumsOfEachPage;
};

// page  structure
// (RM_PageHeader, bitmap, slot ... slot, free space)
// nextFree could be PageStatus or positive number(stand for next free page)
// size of bitmap is depend on RM_FileHeader.recordNumsOfEachPage, use marco BITMAP_SIZE can compute it
// bits in bitmap: 1 starnd for slot is used, 0 stand for slot is free
struct RM_PageHeader {
    int nextFree;
    char bitmap;

    static inline int size(int recordNums) { return sizeof(int) + BitMapWapper::memorySize(recordNums); }
};

#endif