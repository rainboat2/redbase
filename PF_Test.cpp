#include "PF_BufferManager.h"
#include "PF_BufferStrategy.h"
#include "pf.h"
#include "gtest/gtest.h"
#include <array>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

class LRUTest : public testing::Test {
protected:
    void SetUp() override
    {
        LRU_ = new LRU<BufferKey, BufferKeyHash>();
        keys[0] = { 0, 0 };
        keys[1] = { 0, 2147483647 };
        keys[2] = { 2147483647, 0 };
        for (int i = 3; i < keys.size(); i++) {
            keys[i] = { i, i };
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
    for (int i = 1; i < keys.size() - 1; i++) {
        LRU_->visit(keys[i]);
    }
    EXPECT_EQ(LRU_->pop(), keys.back());
    for (int i = 2; i < keys.size() - 1; i++) {
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
    const char* TEST_FILE = "/tmp/PF_CREATE_TEST";
};

TEST_F(PF_ManagerTest, PF_MANAGER_FILE_MANAGER)
{
    RC rc = manager_.CreateFile(TEST_FILE);
    if (rc != RC::PF_SUCCESSS)
        PF_PrintError(rc);
    EXPECT_EQ(rc, RC::PF_SUCCESSS);
    EXPECT_EQ(0, access(TEST_FILE, AT_EACCESS));
    
    PF_FileHandle handle;
    EXPECT_EQ(RC::PF_SUCCESSS, manager_.OpenFile(TEST_FILE, handle));
    EXPECT_EQ(RC::PF_SUCCESSS, manager_.CloseFile(handle));

    rc = manager_.DestroyFile(TEST_FILE);
    if (rc != RC::PF_SUCCESSS)
        PF_PrintError(rc);
    EXPECT_EQ(rc, RC::PF_SUCCESSS);
    EXPECT_EQ(-1, access(TEST_FILE, AT_EACCESS));
}

TEST_F(PF_ManagerTest, PF_MANAGER_BLOCK)
{
    char* buffers[PF_BUFFER_SIZE];

    // allocateBlock and disposeBlock for many times
    for (int i = 0; i < 10; i++) {
        for (int i = 0; i < PF_BUFFER_SIZE; i++) {
            EXPECT_EQ(RC::PF_SUCCESSS, manager_.AllocateBlock(buffers[i]));
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
            EXPECT_EQ(RC::PF_SUCCESSS, manager_.DisposeBlock(buffers[i]));
            buffers[i] = nullptr;
        }
    }
}
