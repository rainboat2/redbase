#include "PF_BufferManager.h"
#include "PF_BufferStrategy.h"
#include "PF_Internal.h"
#include "pf.h"
#include <array>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <iostream>
#include <stack>
#include <stdlib.h>
#include <unistd.h>

class LRUTest : public testing::Test {
protected:
    void SetUp() override
    {
        LRU_ = new LRU<BufferKey, BufferKeyHash>();
        keys[0] = { 0, 0 };
        keys[1] = { 0, 2147483647 };
        keys[2] = { 2147483647, 0 };
        for (size_t i = 3; i < keys.size(); i++) {
            keys[i] = { (int)i, (int)i };
        }
    }

    void pushAllKeys()
    {
        for (auto& key : keys)
            LRU_->push(key);
    }

    void TearDown() override
    {
        delete LRU_;
    }

protected:
    BufferStrategy<BufferKey>* LRU_;
    std::array<BufferKey, 40> keys;
};

TEST_F(LRUTest, LRU_PUSH_TEST)
{
    EXPECT_TRUE(LRU_->empty());
    pushAllKeys();
    EXPECT_EQ(LRU_->size(), keys.size());
}

TEST_F(LRUTest, LRU_POP_TEST)
{
    pushAllKeys();
    EXPECT_EQ(LRU_->pop(), keys[0]);
    for (size_t i = 1; i < keys.size() - 1; i++) {
        LRU_->visit(keys[i]);
    }
    EXPECT_EQ(LRU_->pop(), keys.back());
    for (size_t i = 2; i < keys.size() - 1; i++) {
        LRU_->visit(keys[i]);
    }
    EXPECT_EQ(LRU_->pop(), keys[1]);
}

TEST_F(LRUTest, LRU_CONTAIN_REMOVE_TEST)
{
    LRU_->push(keys[0]);
    EXPECT_TRUE(LRU_->contain(keys[0]));
    LRU_->remove(keys[0]);
    EXPECT_FALSE(LRU_->contain(keys[0]));
    EXPECT_TRUE(LRU_->empty());
}

class PF_ManagerTest : public testing::Test {
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

protected:
    PF_Manager manager_;
    const char* TEST_FILE_ = "/tmp/PF_MANAGER_TEST";
};

TEST_F(PF_ManagerTest, PF_MANAGER_FILE_MANAGER)
{
    RC rc = manager_.CreateFile(TEST_FILE_);
    if (rc != RC::SUCCESS)
        PrintError(rc);
    EXPECT_EQ(rc, RC::SUCCESS);
    EXPECT_EQ(0, access(TEST_FILE_, AT_EACCESS));

    PF_FileHandle handle;
    EXPECT_EQ(RC::SUCCESS, manager_.OpenFile(TEST_FILE_, handle));
    EXPECT_EQ(RC::SUCCESS, manager_.CloseFile(handle));

    rc = manager_.DestroyFile(TEST_FILE_);
    if (rc != RC::SUCCESS)
        PrintError(rc);
    EXPECT_EQ(rc, RC::SUCCESS);
    EXPECT_EQ(-1, access(TEST_FILE_, AT_EACCESS));
}

TEST_F(PF_ManagerTest, PF_MANAGER_BLOCK)
{
    char* buffers[PF_BUFFER_SIZE];

    // allocateBlock and disposeBlock for many times
    for (int i = 0; i < 10; i++) {
        for (int i = 0; i < PF_BUFFER_SIZE; i++) {
            EXPECT_EQ(RC::SUCCESS, manager_.AllocateBlock(buffers[i]));
        }
        char* overflow;
        EXPECT_EQ(RC::PF_NOBUF, manager_.AllocateBlock(overflow));
        EXPECT_EQ(RC::PF_NOBUF, manager_.AllocateBlock(overflow));

        // try to access buffer, it may fail if the buffer memory invalid
        for (int i = 0; i < PF_BUFFER_SIZE; i++) {
            char* buffer = buffers[i];
            for (int j = 0; j < PF_FILE_BLOCK_SIZE; j++) {
                buffer[j] = j;
            }
        }

        for (int i = 0; i < PF_BUFFER_SIZE; i++) {
            EXPECT_EQ(RC::SUCCESS, manager_.DisposeBlock(buffers[i]));
            buffers[i] = nullptr;
        }
    }
}

TEST_F(PF_ManagerTest, PF_MANAGER_PRESISTENCE)
{
    manager_.CreateFile(TEST_FILE_);

    PF_FileHandle fileHandle;
    manager_.OpenFile(TEST_FILE_, fileHandle);

    PF_PageHandle p0, p1;
    fileHandle.AllocatePage(p0);
    fileHandle.AllocatePage(p1);

    PageNum pageNum;
    p0.GetPageNum(pageNum);
    fileHandle.MarkDirty(pageNum);
    fileHandle.UnpinPage(pageNum);
    p1.GetPageNum(pageNum);
    fileHandle.MarkDirty(pageNum);
    fileHandle.UnpinPage(pageNum);

    manager_.CloseFile(fileHandle);

    manager_.OpenFile(TEST_FILE_, fileHandle);
    EXPECT_EQ(RC::SUCCESS, fileHandle.GetThisPage(0, p0));
    EXPECT_EQ(RC::SUCCESS, fileHandle.GetThisPage(1, p1));
    manager_.DestroyFile(TEST_FILE_);
}

class PF_FileHandleTest : public testing::Test {
protected:
    void SetUp() override
    {
        manager_.CreateFile(TEST_FILE_);
        manager_.OpenFile(TEST_FILE_, handle_);
    }

    void TearDown() override
    {
        manager_.CloseFile(handle_);
        manager_.DestroyFile(TEST_FILE_);
    }

protected:
    PF_Manager manager_;
    PF_FileHandle handle_;
    const char* TEST_FILE_ = "/tmp/PF_FILE_HANDLE_TEST";
};

TEST_F(PF_FileHandleTest, PAGE_ALLOCAGE_AND_DISPOSE)
{
    for (int i = 0; i < 2; i++) {
        PF_PageHandle pageHandle;
        EXPECT_EQ(RC::SUCCESS, handle_.AllocatePage(pageHandle));
        PageNum pageNum;
        pageHandle.GetPageNum(pageNum);
        EXPECT_EQ(0, pageNum);
        EXPECT_EQ(RC::SUCCESS, handle_.DisposePage(pageNum));
    }
    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        PF_PageHandle pageHandle;
        EXPECT_EQ(RC::SUCCESS, handle_.AllocatePage(pageHandle));
    }

    std::vector<int> delete_seq { 0, 19, 1, 18 };
    std::stack<int> free_page;
    for (auto d : delete_seq) {
        handle_.DisposePage(d);
        free_page.push(d);
    }

    for (int i = 0; i < 4; i++) {
        PF_PageHandle pageHandle;
        EXPECT_EQ(RC::SUCCESS, handle_.AllocatePage(pageHandle));
        PageNum pageNum;
        pageHandle.GetPageNum(pageNum);
        EXPECT_EQ(pageNum, free_page.top());
        free_page.pop();
    }
    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        handle_.UnpinPage(i);
    }

    {
        PF_PageHandle pageHandle;
        EXPECT_EQ(RC::SUCCESS, handle_.AllocatePage(pageHandle));
        PageNum pageNum;
        pageHandle.GetPageNum(pageNum);
    }
}

TEST_F(PF_FileHandleTest, PAGE_READ_WRITE_TEST)
{
    for (int i = 0; i < PF_BUFFER_SIZE; i++) {
        PF_PageHandle pageHandle;
        PageNum pageNum;
        char* data;
        EXPECT_EQ(RC::SUCCESS, handle_.AllocatePage(pageHandle));
        pageHandle.GetPageNum(pageNum);
        pageHandle.GetData(data);
        memset(data, i, PF_PAGE_SIZE);
        handle_.MarkDirty(pageNum);
        handle_.UnpinPage(pageNum);
    }
    // system("xxd /tmp/PF_FILE_HANDLE_TEST > tmp.txt");

    {
        PF_PageHandle pageHandle;
        PageNum pageNum;
        char* data;
        handle_.GetFirstPage(pageHandle);
        pageHandle.GetPageNum(pageNum);
        pageHandle.GetData(data);
        for (size_t i = 0; i < PF_PAGE_SIZE; i++)
            EXPECT_EQ(0, data[i]);

        for (size_t i = 1; i < PF_BUFFER_SIZE; i++) {
            EXPECT_EQ(RC::SUCCESS, handle_.GetNextPage(pageNum, pageHandle));
            pageHandle.GetPageNum(pageNum);
            pageHandle.GetData(data);
            for (unsigned long j = 0; j < PF_PAGE_SIZE; j++)
                EXPECT_EQ(i, data[j]);
        }
        EXPECT_EQ(RC::PF_EOF, handle_.GetNextPage(pageNum, pageHandle));
    }

    {
        PF_PageHandle pageHandle;
        PageNum pageNum;
        char* data;
        handle_.GetLastPage(pageHandle);
        pageHandle.GetPageNum(pageNum);
        pageHandle.GetData(data);
        for (int i = PF_PAGE_SIZE - 1; i >= 0; i--)
            EXPECT_EQ(PF_BUFFER_SIZE - 1, data[i]);

        for (int i = PF_BUFFER_SIZE - 2; i >= 0; i--) {
            EXPECT_EQ(RC::SUCCESS, handle_.GetPrevPage(pageNum, pageHandle));
            pageHandle.GetPageNum(pageNum);
            pageHandle.GetData(data);
            for (unsigned long j = 0; j < PF_PAGE_SIZE; j++)
                EXPECT_EQ(i, data[j]);
        }
    }
}

class PF_BufferManagerTest: public testing::Test{
protected:
    void SetUp() override {
        fd_ = open(TEST_FILE_, O_CREAT | O_RDWR);
        char buffer[PF_PAGE_SIZE];

        for (int i = 0; i < PF_BUFFER_SIZE * 5; i++){
            memset(buffer, i, PF_PAGE_SIZE);
            write(fd_, buffer, PF_PAGE_SIZE);
        }
    }
    void TearDown() override {
        close(fd_);
        unlink(TEST_FILE_);
    }

protected:
    int fd_;
    const char* TEST_FILE_ = "/tmp/PF_BUFFER_MANAGER_TEST";
    PF_BufferManager bufferManager_;
};

TEST_F(PF_BufferManagerTest, BUFFER_TEST){
    for (int i = 0; i < PF_BUFFER_SIZE * - 1; i++){
        char *data;
        RC rc =bufferManager_.ReadPage(fd_, i, data);
        EXPECT_EQ(rc, RC::SUCCESS);
        rc = bufferManager_.MarkDirty(fd_, i);
        EXPECT_EQ(rc, RC::SUCCESS);
        rc = bufferManager_.UnpinPage(fd_, i);
        EXPECT_EQ(rc, RC::SUCCESS);
    }
}