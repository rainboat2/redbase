#ifndef BITMAPWAPPER_HH
#define BITMAPWAPPER_HH

#include <stddef.h>

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

    // count how many bytes is needed for specified bitsNum_
    inline static size_t memorySize(size_t bitsNum_) { return bitsNum_ / 8 + ((bitsNum_ % 8 == 0) ? 0 : 1); }

private:
    // record bits from left to right
    unsigned char* bitmap_;
    size_t bitsNum_;
    size_t bitmap_size_;
};

#endif