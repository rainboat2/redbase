#include "RM_Internal.h"


BitMapWapper::BitMapWapper(char* bitmap, size_t bitsNum)
    :bitsNum_(bitsNum)
{
    bitmap_ = (unsigned char*) bitmap;
    bitmap_size_ = BITMAP_SIZE(bitsNum);
}

void BitMapWapper::set(size_t i, bool b)
{
    size_t charIndex = i / 8;
    size_t pos = i % 8;
    if (b) bitmap_[charIndex] = bitmap_[charIndex] | 0b10000000 >> pos;
    else   bitmap_[charIndex] = bitmap_[charIndex] & ~(0b10000000 >> pos);
}

bool BitMapWapper::get(size_t i) const
{
    int charIndex = i / 8;
    int pos = i % 8;
    return bitmap_[charIndex] & 0b10000000 >> pos;
}

bool BitMapWapper::all() const
{
    constexpr unsigned char all_one = 0b11111111;
    unsigned char rs = all_one;
    for (size_t i = 0; i < bitmap_size_ - 1; i++) {
        rs &= bitmap_[i];
        if (rs != all_one) break;
    }
    unsigned char last = all_one >> bitsNum_ % 8;
    last |= bitmap_[bitmap_size_ - 1];
    rs &= last;
    return rs == all_one;
}

int BitMapWapper::findFirstZero() const{
    constexpr unsigned char all_one = 0b11111111;
    size_t bitIndex= 0;
    while ((bitIndex / 8) < (bitmap_size_ - 1) && bitmap_[bitIndex / 8] == all_one)
        bitIndex += 8;
    while (bitIndex < bitsNum_ && get(bitIndex) == true)
        bitIndex++;
    return ((bitIndex == bitsNum_)? -1 : bitIndex);
}