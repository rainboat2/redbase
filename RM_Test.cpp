#include "RM_Internal.h"
#include "rm.h"

#include <gtest/gtest.h>
#include <unordered_set>

class BitMapWapperTest : public testing::Test {
protected:
    void SetUp() override
    {
        memset(b1_buf, 0, 1);
        memset(b7_buf, 0, 1);
        memset(b16_buf, 0, 2);
        memset(b31_buf, 0, 4);
    }

protected:
    char b1_buf[1];
    char b7_buf[1];
    char b16_buf[2];
    char b31_buf[4];
};

TEST_F(BitMapWapperTest, BITMAPTEST)
{
    BitMapWapper b1_(b1_buf, 1), b7_(b7_buf, 7), b16_(b16_buf, 16), b31_(b31_buf, 31);

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

TEST_F(BitMapWapperTest, BITMAP_CREATE_GET_TEST)
{
    char buf1[4] { 0, 0, 0, 0 };
    BitMapWapper b1(buf1, 27);
    std::unordered_set<int> s1 { 1, 5, 11, 8, 23 };
    for (int i : s1)
        b1.set(i, false);

    char buf2[4];
    memcpy(buf2, buf1, 4);
    BitMapWapper b2(buf2, b1.size());
    for (int i = 0; i < b1.size(); i++){
        EXPECT_EQ(b1.get(i), b2.get(i));
    }
}