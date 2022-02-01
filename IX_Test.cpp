#include <array>
#include <gtest/gtest.h>
#include <iostream>
#include <unistd.h>

#include "ix.h"
#include "pf.h"
#include "rm.h"

class IX_BNodeWapperTest : public testing::Test {
protected:
    void SetUp() override { }
    void TearDown() override { }
};

TEST_F(IX_BNodeWapperTest, INDEX_OF_TEST)
{
    char buffer[PF_PAGE_SIZE];
    std::array<int, 5> arr { 1, 2, 2, 4, 8 };
    *((int*)buffer) = arr.size();
    int* attr = (int*)buffer + 1;
    for (int i = 0; i < arr.size(); i++) {
        attr[i] = arr[i];
    }
    IX_BNodeWapper node(sizeof(int), AttrType::RD_INT, buffer, { -1, -1 });
    EXPECT_EQ(node.size(), arr.size());

    const std::map<int, int> upperBound = { { -1, 0 }, { 0, 0 }, { 1, 1 }, { 2, 3 }, { 3, 3 }, { 4, 4 }, { 5, 4 }, { 8, 5 }, { 12, 5 } };
    for (auto pair : upperBound) {
        EXPECT_EQ(node.upperBound(&pair.first), pair.second);
    }

    const std::map<int, int> lowerBound = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 2, 0 }, { 3, 2 }, { 4, 2 }, { 8, 3 }, { 12, 4 } };
    for (auto pair : lowerBound) {
        EXPECT_EQ(node.lowerBound(&pair.first), pair.second);
    }

    const std::map<int, int> indexOf = { { -1, -1 }, { 0, -1 }, { 1, 0 }, { 2, 1 }, { 3, -1 }, { 4, 3 }, { 8, 4 }, { 12, -1 } };
    for (auto pair : indexOf) {
        EXPECT_EQ(node.indexOf(&pair.first), pair.second);
    }
}

TEST_F(IX_BNodeWapperTest, LEAF_INSERT_TEST)
{
    char buffer[PF_PAGE_SIZE];
    memset(buffer, 0, PF_PAGE_SIZE);
    IX_BNodeWapper node(sizeof(int), AttrType::RD_INT, buffer, { -1, -1 });
    EXPECT_EQ(0, node.size());
    std::array<int, 340> seq;
    for (int i = 0; i < seq.size(); i++)
        seq[i] = i;
    std::reverse(seq.begin(), seq.end());
    for (int i = 0; i < seq.size(); i++)
        node.leafInsert(&seq[i], { seq[i], seq[i] });

    for (int i = 0; i < seq.size(); i++) {
        RID rid = node.getRid(i);
        void* attr = node.getAttr(i);
        PageNum pageNum;
        rid.GetPageNum(pageNum);
        EXPECT_EQ(pageNum, i);
        EXPECT_EQ(*((int*)attr), i);
    }
    EXPECT_TRUE(node.isFull());
}

TEST_F(IX_BNodeWapperTest, NOT_LEAF_INSERT_TEST)
{
    char buffer[PF_PAGE_SIZE];
    memset(buffer, 0, PF_PAGE_SIZE);
    IX_BNodeWapper node(sizeof(int), AttrType::RD_INT, buffer, { -1, -1 });
    node.setRid(0, { -1, -1 });
    for (int i = 0; i < IX_BNodeWapper::countOrder(sizeof(int)); i++) {
        IX_BInsertUpEntry up { true };
        memcpy(up.attr, &i, sizeof(int));
        up.right = { i, i };
        node.notLeafInsert(up);
    }
    EXPECT_TRUE(node.isFull());
}

TEST_F(IX_BNodeWapperTest, LEAF_SPILT_TEST)
{
    char b1[PF_PAGE_SIZE];
    memset(b1, 0, PF_PAGE_SIZE);
    IX_BNodeWapper node(sizeof(int), AttrType::RD_INT, b1, { -1, -1 });
    for (int i = 0; i < IX_BNodeWapper::countOrder(sizeof(int)); i++) {
        node.leafInsert(&i, { i, i });
    }

    char b2[PF_PAGE_SIZE];
    memset(b2, 0, PF_PAGE_SIZE);
    int fullSize = node.size();
    IX_BNodeWapper newNode(sizeof(int), AttrType::RD_INT, b2, { -1, -1 });
    int tmp = 450;
    EXPECT_TRUE(node.isFull());
    auto cur = node.leafSpiltAndInsert(&tmp, { tmp, tmp }, newNode);
    EXPECT_TRUE(cur.isSpilt);
    EXPECT_EQ(node.size(), (fullSize + 1) / 2);
    EXPECT_EQ(newNode.size(), (fullSize + 1) / 2 + 1);
}

TEST_F(IX_BNodeWapperTest, NOT_LEAF_SPLIT_TEST)
{
    char b1[PF_PAGE_SIZE];
    memset(b1, 0, PF_PAGE_SIZE);
    IX_BNodeWapper node(sizeof(int), AttrType::RD_INT, b1, { -1, -1 });
    node.setRid(0, { -1, -1 });
    for (int i = 0; i < IX_BNodeWapper::countOrder(sizeof(int)); i++) {
        IX_BInsertUpEntry up { true };
        memcpy(up.attr, &i, sizeof(int));
        up.right = { i, i };
        node.notLeafInsert(up);
    }
    int fullSize = node.size();
    EXPECT_TRUE(node.isFull());
    char b2[PF_PAGE_SIZE];
    memset(b2, 0, PF_PAGE_SIZE);
    IX_BNodeWapper newNode(sizeof(int), AttrType::RD_INT, b2, { -1, -1 });

    IX_BInsertUpEntry up { true };
    int tmp = 341;
    memcpy(up.attr, &tmp, sizeof(int));
    up.right = { tmp, tmp };
    auto cur = node.notLeafSpiltAndInsert(up, newNode);
    EXPECT_TRUE(cur.isSpilt);
    EXPECT_EQ(node.size(), (fullSize + 1) / 2);
    EXPECT_EQ(newNode.size(), (fullSize + 1) / 2);
}

class IX_ManagerTest : public testing::Test {
protected:
    void SetUp() override
    {
        pf_manager_ = new PF_Manager;
        ix_manager_ = new IX_Manager(*pf_manager_);
    }

    void TearDown() override
    {
        delete ix_manager_;
        delete pf_manager_;
    }

protected:
    IX_Manager* ix_manager_;
    PF_Manager* pf_manager_;
    const char* TEST_FILE_ = "/tmp/IX_MANAGER_HANDLE_TEST_";
    const int INDEX_NO_ = 2233;
    const AttrType attrType_ = AttrType::RD_INT;
    const int attrLength_ = sizeof(int);
};

TEST_F(IX_ManagerTest, IX_CREAT_DESTORY_TEST)
{
    auto name = ix_manager_->getFileName(TEST_FILE_, INDEX_NO_);

    RC rc = ix_manager_->CreateIndex(TEST_FILE_, INDEX_NO_, attrType_, attrLength_);
    EXPECT_EQ(rc, RC::SUCCESSS);
    EXPECT_EQ(0, access(name.c_str(), F_OK));

    rc = ix_manager_->DestroyIndex(TEST_FILE_, INDEX_NO_);
    EXPECT_EQ(rc, RC::SUCCESSS);
    EXPECT_NE(0, access(name.c_str(), F_OK));
}

TEST_F(IX_ManagerTest, IX_OPEN_AND_CLOSE_TEST)
{
    ix_manager_->CreateIndex(TEST_FILE_, INDEX_NO_, attrType_, attrLength_);

    IX_IndexHandle indexHandle;
    RC rc = ix_manager_->OpenIndex(TEST_FILE_, INDEX_NO_, indexHandle);
    EXPECT_EQ(rc, RC::SUCCESSS);

    rc = ix_manager_->CloseIndex(indexHandle);
    EXPECT_EQ(rc, RC::SUCCESSS);

    ix_manager_->DestroyIndex(TEST_FILE_, INDEX_NO_);
}

class IX_IndexHandleTest : public testing::Test {
protected:
    void SetUp() override
    {
        pf_manager_ = new PF_Manager;
        ix_manager_ = new IX_Manager(*pf_manager_);
        ix_manager_->CreateIndex(TEST_FILE_, INDEX_NO_, attrType_, attrLength_);
        ix_manager_->OpenIndex(TEST_FILE_, INDEX_NO_, indexHandle_);
    }

    void TearDown() override
    {
        ix_manager_->CloseIndex(indexHandle_);
        ix_manager_->DestroyIndex(TEST_FILE_, INDEX_NO_);
        delete ix_manager_;
        delete pf_manager_;
    }

protected:
    PF_Manager* pf_manager_;
    IX_Manager* ix_manager_;
    IX_IndexHandle indexHandle_;
    const char* TEST_FILE_ = "/tmp/IX_INDEX_HANDLE_TEST_";
    const int INDEX_NO_ = 2233;
    const AttrType attrType_ = AttrType::RD_INT;
    const int attrLength_ = sizeof(int);
};

TEST_F(IX_IndexHandleTest, IX_INDEX_INSERT_TEST)
{
    for (int i = 10000; i >= 0; i--) {
        RC rc = indexHandle_.InsertEntry(&i, { i, i });
        EXPECT_EQ(rc, RC::SUCCESSS);
    }
}

TEST_F(IX_IndexHandleTest, IX_INDEX_DELETE_TEST)
{
    for (int i = 10000; i >= 0; i--) {
        RC rc = indexHandle_.InsertEntry(&i, { i, i });
        EXPECT_EQ(rc, RC::SUCCESSS);
    }
    int target = 300;
    EXPECT_EQ(RC::SUCCESSS, indexHandle_.DeleteEntry(&target, {target, target}));
}

class IX_IndexScanTest : public testing::Test {
protected:
    void SetUp() override
    {
        pf_manager_ = new PF_Manager;
        ix_manager_ = new IX_Manager(*pf_manager_);
        ix_manager_->CreateIndex(TEST_FILE_, INDEX_NO_, attrType_, attrLength_);
        ix_manager_->OpenIndex(TEST_FILE_, INDEX_NO_, indexHandle_);
    }

    void constructDefaultIndex(int entryNum)
    {
        for (int i = entryNum - 1; i >= 0; i--) {
            indexHandle_.InsertEntry(&i, { i, i });
        }
    }

    void TearDown() override
    {
        ix_manager_->CloseIndex(indexHandle_);
        ix_manager_->DestroyIndex(TEST_FILE_, INDEX_NO_);
        delete ix_manager_;
        delete pf_manager_;
    }

protected:
    PF_Manager* pf_manager_;
    IX_Manager* ix_manager_;
    IX_IndexHandle indexHandle_;
    const char* TEST_FILE_ = "/tmp/IX_INDEX_SCAN_TEST";
    const int INDEX_NO_ = 2233;
    const AttrType attrType_ = AttrType::RD_INT;
    const int attrLength_ = sizeof(int);
    const int ENTRY_SIZE_ = 1000;
};

TEST_F(IX_IndexScanTest, IX_INDEX_SCAN_EQ_TEST)
{
    constructDefaultIndex(ENTRY_SIZE_);
    IX_IndexScan scan;
    int target = ENTRY_SIZE_ / 2;
    EXPECT_EQ(RC::SUCCESSS, scan.OpenScan(indexHandle_, CompOp::EQ, &target));
    RID rid;
    int cnt = 0;
    while (scan.GetNextEntry(rid) == RC::SUCCESSS) {
        EXTRACT_PAGE_NUM(pageNum, rid);
        EXPECT_EQ(target, pageNum);
        cnt++;
    }
    EXPECT_EQ(cnt, 1);
    EXPECT_EQ(RC::SUCCESSS, scan.CloseScan());
}

TEST_F(IX_IndexScanTest, IX_INDEX_SCAN_NE_TEST)
{
    constructDefaultIndex(ENTRY_SIZE_);
    IX_IndexScan scan;
    int target = ENTRY_SIZE_ / 2;
    EXPECT_EQ(RC::SUCCESSS, scan.OpenScan(indexHandle_, CompOp::NE, &target));
    RID rid;
    int cnt = 0;
    while (scan.GetNextEntry(rid) == RC::SUCCESSS) {
        while (cnt == target)
            cnt++;
        EXTRACT_PAGE_NUM(pageNum, rid);
        EXPECT_EQ(cnt, pageNum);
        cnt++;
    }
    EXPECT_EQ(cnt, ENTRY_SIZE_);
    EXPECT_EQ(RC::SUCCESSS, scan.CloseScan());
}

TEST_F(IX_IndexScanTest, IX_INDEX_SCAN_GE_TEST)
{
    constructDefaultIndex(ENTRY_SIZE_);
    IX_IndexScan scan;
    int target = ENTRY_SIZE_ / 2;
    EXPECT_EQ(RC::SUCCESSS, scan.OpenScan(indexHandle_, CompOp::GE, &target));
    RID rid;
    int cnt = 0;
    while (scan.GetNextEntry(rid) == RC::SUCCESSS) {
        EXTRACT_PAGE_NUM(pageNum, rid);
        EXPECT_GE(pageNum, target);
        cnt++;
    }
    EXPECT_EQ(cnt, ENTRY_SIZE_ - target);
    EXPECT_EQ(RC::SUCCESSS, scan.CloseScan());
}

TEST_F(IX_IndexScanTest, IX_INDEX_SCAN_GT_TEST)
{
    constructDefaultIndex(ENTRY_SIZE_);
    IX_IndexScan scan;
    int target = ENTRY_SIZE_ / 2;
    EXPECT_EQ(RC::SUCCESSS, scan.OpenScan(indexHandle_, CompOp::GT, &target));
    RID rid;
    int cnt = 0;
    while (scan.GetNextEntry(rid) == RC::SUCCESSS) {
        EXTRACT_PAGE_NUM(pageNum, rid);
        EXPECT_GT(pageNum, target);
        cnt++;
    }
    EXPECT_EQ(cnt, ENTRY_SIZE_ - target - 1);
    EXPECT_EQ(RC::SUCCESSS, scan.CloseScan());
}

TEST_F(IX_IndexScanTest, IX_INDEX_SCAN_LE_TEST)
{
    constructDefaultIndex(ENTRY_SIZE_);
    IX_IndexScan scan;
    int target = ENTRY_SIZE_ / 2;
    EXPECT_EQ(RC::SUCCESSS, scan.OpenScan(indexHandle_, CompOp::LE, &target));
    RID rid;
    int cnt = 0;
    while (scan.GetNextEntry(rid) == RC::SUCCESSS) {
        EXTRACT_PAGE_NUM(pageNum, rid);
        EXPECT_LE(pageNum, target);
        cnt++;
    }
    EXPECT_EQ(cnt, target + 1);
    EXPECT_EQ(RC::SUCCESSS, scan.CloseScan());
}

TEST_F(IX_IndexScanTest, IX_INDEX_SCAN_LT_TEST)
{
    constructDefaultIndex(ENTRY_SIZE_);
    IX_IndexScan scan;
    int target = ENTRY_SIZE_ / 2;
    EXPECT_EQ(RC::SUCCESSS, scan.OpenScan(indexHandle_, CompOp::LT, &target));
    RID rid;
    int cnt = 0;
    while (scan.GetNextEntry(rid) == RC::SUCCESSS) {
        EXTRACT_PAGE_NUM(pageNum, rid);
        EXPECT_LE(pageNum, target);
        cnt++;
    }
    EXPECT_EQ(cnt, target);
    EXPECT_EQ(RC::SUCCESSS, scan.CloseScan());
}

TEST_F(IX_IndexScanTest, IX_INDEX_SCAN_NO_TEST)
{
    constructDefaultIndex(ENTRY_SIZE_);
    IX_IndexScan scan;
    EXPECT_EQ(RC::SUCCESSS, scan.OpenScan(indexHandle_, CompOp::NO, nullptr));
    RID rid;
    int cnt = 0;
    while (scan.GetNextEntry(rid) == RC::SUCCESSS) {
        EXTRACT_PAGE_NUM(pageNum, rid);
        EXPECT_EQ(cnt, pageNum);
        cnt++;
    }
    EXPECT_EQ(cnt, ENTRY_SIZE_);
    EXPECT_EQ(RC::SUCCESSS, scan.CloseScan());
}