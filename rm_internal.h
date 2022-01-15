#ifndef RM_INTERNAL_HH
#define RM_INTERNAL_HH

#include <string.h>

using SlotNum = int;

struct RM_FileHeader {
    int pageNums;
    int nextFree;
    int recordSize;
    int recordNumsOfEachPage;
};

#define BITMAP_SIZE(recordNums) \
    (recordNums / 8 + ((recordNums % 8 == 0) ? 0 : 1))
// page  structure
// (RM_PageHeader, bitmap, slot ... slot, free space)
// size of bitmap is depend on RM_FileHeader.recordNumsOfEachPage, use marco BITMAP_SIZE can compute it
// bits in bitmap: 1 starnd for slot is used, 0 stand for slot is free
struct RM_PageHeader {
    int nextFree;
    char* bitmap;

    RM_PageHeader(int recordNums, char* buf)
    {
        memcpy(&nextFree, buf, sizeof(int));
        int bitMapSize = BITMAP_SIZE(recordNums);
        bitmap = new char[bitMapSize];
        memcpy(bitmap, buf + sizeof(int), bitMapSize);
    }

    ~RM_PageHeader()
    {
        delete bitmap;
    }

    void to_buffer(int recordNums, char* buf)
    {
        memcpy(buf, &nextFree, sizeof(int));
        memcpy(buf, bitmap + sizeof(int), BITMAP_SIZE(recordNums));
    }
};

class BitMap {
public:
    BitMap();
    BitMap(char* data, int bitsNum);
    BitMap(const BitMap& b);
    BitMap& operator=(const BitMap& b);
    
    ~BitMap();

    inline int size() const {return bitsNum_;};
    void set(int i, bool b);
    bool get(int i) const;
    // true if all bits is set to 1, else return false
    bool all() const;

    void toBuf(char *buf) const;

private:
    // record bits from left to right
    unsigned char* bitmap_;
    int bitsNum_;
};

#endif