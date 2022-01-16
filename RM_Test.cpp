#include "RM_Internal.h"
#include "rm.h"

#include <gtest/gtest.h>
#include <unordered_set>

class BitMapTest : public testing::Test {
protected:
    void SetUp() override
    {
        char buf[4] { 0, 0, 0, 0 };
        b1_ = BitMap(buf, 1);
        b7_ = BitMap(buf, 7);
        b16_ = BitMap(buf, 16);
        b31_ = BitMap(buf, 31);
    }

protected:
    BitMap b1_;
    BitMap b7_;
    BitMap b16_;
    BitMap b31_;
};

TEST_F(BitMapTest, BITMAPTEST)
{
    EXPECT_EQ(false, b1_.all());
    b1_.set(0, true);
    EXPECT_EQ(true, b1_.all());
    EXPECT_EQ(true, b1_.get(0));

    b7_.set(6, true);
    EXPECT_EQ(true, b7_.get(6));

    EXPECT_EQ(16, b16_.size());
    for (size_t i = 0; i < b16_.size(); i++)
        b16_.set(i, true);
    EXPECT_EQ(true, b16_.all());
    std::unordered_set<size_t> s1 { 1, 5, 11, 8 };
    for (size_t i : s1)
        b16_.set(i, false);
    for (size_t i = 0; i < b16_.size(); i++) {
        if (s1.find(i) != s1.end())
            EXPECT_EQ(false, b16_.get(i));
        else
            EXPECT_EQ(true, b16_.get(i));
    }

    std::unordered_set<int> s2 { 3, 6, 25 };
    for (size_t i : s2) {
        b31_.set(i, true);
    }
    for (size_t i = 0; i < s2.size(); i++) {
        if (s2.find(i) == s2.end())
            EXPECT_EQ(false, b31_.get(i));
        else
            EXPECT_EQ(true, b31_.get(i));
    }
}

TEST_F(BitMapTest, BITMAP_CREATE_GET_TEST)
{
    char buf[4] { 0, 0, 0, 0 };
    BitMap b1(buf, 27);
    std::unordered_set<int> s1 { 1, 5, 11, 8, 23 };
    for (int i : s1)
        b1.set(i, false);
    b1.toBuf(buf);
    BitMap b2(buf, b1.size());
    for (int i = 0; i < b1.size(); i++){
        EXPECT_EQ(b1.get(i), b2.get(i));
    }
}