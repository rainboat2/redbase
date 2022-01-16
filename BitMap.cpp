#include "RM_Internal.h"

BitMap::BitMap()
    : bitsNum_(8)
{
    bitmap_ = new unsigned char[1];
    bitmap_[0] = 0;
}

BitMap::BitMap(char* data, size_t bitsNum)
    : bitsNum_(bitsNum)
{
    int bitmapSize = BITMAP_SIZE(bitsNum_);
    bitmap_ = new unsigned char[bitmapSize];
    memcpy(bitmap_, data, bitmapSize);
}

BitMap::BitMap(const BitMap& b)
{
    bitsNum_ = b.bitsNum_;
    int bitmapSize = BITMAP_SIZE(bitsNum_);
    bitmap_ = new unsigned char[bitmapSize];
    memcpy(bitmap_, b.bitmap_, bitmapSize);
}

BitMap& BitMap::operator=(const BitMap& b){
    if (&b != this){
        int newBitmapSize = BITMAP_SIZE(b.bitsNum_);
        if (BITMAP_SIZE(bitsNum_) != newBitmapSize){
            delete[] bitmap_;
            bitmap_ = new unsigned char[newBitmapSize];
        }
        bitsNum_ = b.bitsNum_;
        memcpy(bitmap_, b.bitmap_, newBitmapSize);
    }
    return *this;
}

BitMap::~BitMap()
{
    delete[] bitmap_;
}

void BitMap::set(size_t i, bool b)
{
    size_t charIndex = i / 8;
    size_t pos = i % 8;
    if (b) bitmap_[charIndex] = bitmap_[charIndex] | 0b10000000 >> pos;
    else   bitmap_[charIndex] = bitmap_[charIndex] & ~(0b10000000 >> pos);
}

bool BitMap::get(size_t i) const
{
    int charIndex = i / 8;
    int pos = i % 8;
    return bitmap_[charIndex] & 0b10000000 >> pos;
}

bool BitMap::all() const
{
    constexpr unsigned char all_one = 0b11111111;
    size_t bitmap_size = BITMAP_SIZE(bitsNum_);
    unsigned char rs = all_one;
    for (size_t i = 0; i < bitmap_size - 1; i++) {
        rs &= bitmap_[i];
    }
    unsigned char last = all_one >> bitsNum_ % 8;
    last |= bitmap_[bitmap_size - 1];
    rs &= last;
    return rs == all_one;
}

void BitMap::toBuf(char *buf) const{
    memcpy(buf, bitmap_, BITMAP_SIZE(bitsNum_));
}