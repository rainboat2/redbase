#include "RM_Internal.h"
#include "pf.h"
#include "rm.h"
#include <unistd.h>

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
    std::unordered_set<size_t> s1 { 0, 5, 11, 8 };
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
    EXPECT_EQ(false, b31_.all());
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
    for (int i = 0; i < b1.size(); i++) {
        EXPECT_EQ(b1.get(i), b2.get(i));
    }
}

TEST_F(BitMapWapperTest, BITMAP_FIND_TEST)
{
    memset(b31_buf, 0b11111111, 4);
    BitMapWapper b31(b31_buf, 31);
    EXPECT_EQ(-1, b31.findFirstZero());

    std::vector<size_t> s { 0, 7, 8, 15, 16, 28, 30 };
    b31.set(0, false);
    EXPECT_EQ(b31.all(), false);
    EXPECT_EQ(0, b31.findFirstZero());
    for (size_t i : s) {
        b31.set(i, false);
        EXPECT_EQ(i, b31.findFirstZero());
        b31.set(i, true);
    }
}

class RM_PageHeaderTest : public testing::Test {
protected:
    void SetUp() override
    {
        memset(page1_, 0, PF_PAGE_SIZE);
        memset(page2_, 0, PF_PAGE_SIZE);
    }

    void TearDown() override { }

protected:
    char page1_[PF_PAGE_SIZE];
    char page2_[PF_PAGE_SIZE];
    const int recordNums_ = 23;
};

TEST_F(RM_PageHeaderTest, RM_PAGE_TEST){
    RM_PageHeader* pageHdr1 = (RM_PageHeader*) page1_;
    BitMapWapper b1 (&pageHdr1->bitmap, recordNums_);
    std::vector<size_t> seq{0, 7, 8, 3, 22};
    for (auto i : seq)
        b1.set(i, true);
    
    memcpy(page2_, page1_, PF_PAGE_SIZE);
    RM_PageHeader* pageHdr2 = (RM_PageHeader*) page2_;
    BitMapWapper b2 (&pageHdr2->bitmap, recordNums_);
    for (int i = 0; i < recordNums_; i++){
        EXPECT_EQ(b1.get(i), b2.get(i));
    }
}

class RM_ManagerTest : public testing::Test
{
protected:
    void SetUp() override { }

    void TearDown() override { }

protected:
    const char* TEST_FILE_ = "/tmp/rm_manager_test";
    PF_Manager pf_manager_;
    constexpr int record_size_ = 16;
    char data_[record_size_];
};

TEST_F(RM_ManagerTest, RM_MANAGER_TEST){
    RM_Manager manager (pf_manager_);
    EXPECT_EQ(RC::SUCCESSS, manager.CreateFile(TEST_FILE_, record_size_));
    EXPECT_EQ(0, access(TEST_FILE_, F_OK));
    RM_FileHandle fileHandle;
    EXPECT_EQ(RC::SUCCESSS, manager.OpenFile(TEST_FILE_, fileHandle));

    RID rids[PF_PAGE_SIZE];
    for (int i = 0; i < PF_PAGE_SIZE; i++){
        memset(data_, i, record_size_);
        EXPECT_EQ(RC::SUCCESSS, fileHandle.InsertRec(data_, rids[i]));
    }

    for (int i = 0; i < PF_PAGE_SIZE; i++){
        memset(data, i, record_size_);
        RM_Record rec;
    }
}