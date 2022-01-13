#include "PF_BufferStrategy.h"
#include "PF_BufferManager.h"
#include "gtest/gtest.h"
#include <iostream>
#include <array>

class LRUTest : public testing::Test {
protected:
    void SetUp() override{
        LRU_ = new LRU<BufferKey, BufferKeyHash>();
        keys[0] = {0, 0};
        keys[1] = {0, 2147483647};
        keys[2] = {2147483647, 0};
        for (int i = 3; i < keys.size(); i++){
            keys[i] = {i, i};
        }
    }

    void pushAllKeys(){
        for (auto &key: keys)
            LRU_->push(key);
    }

    void TearDown() override{
        delete LRU_;
    } 

protected:
    BufferStrategy<BufferKey>* LRU_;
    std::array<BufferKey, 40> keys;
};

TEST_F(LRUTest, LRU_PUSH_TEST){
    EXPECT_TRUE(LRU_->empty());
    pushAllKeys();
    EXPECT_EQ(LRU_->size(), keys.size());
}

TEST_F(LRUTest, LRU_POP_TEST){
    pushAllKeys();
    EXPECT_EQ(LRU_->pop(), keys[0]);
    for (int i = 1; i < keys.size() - 1; i++){
        LRU_->visit(keys[i]);
    }
    EXPECT_EQ(LRU_->pop(), keys.back());
    for (int i = 2; i < keys.size() - 1; i++){
        LRU_->visit(keys[i]);
    }
    EXPECT_EQ(LRU_->pop(), keys[1]);
}

TEST_F(LRUTest, LRU_CONTAIN_REMOVE_TEST){
    LRU_->push(keys[0]);
    EXPECT_TRUE(LRU_->contain(keys[0]));
    LRU_->remove(keys[0]);
    EXPECT_FALSE(LRU_->contain(keys[0]));
    EXPECT_TRUE(LRU_->empty());
}


class BufferManagerTest : public testing::Test{

}