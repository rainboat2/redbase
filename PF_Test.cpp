#include "PF_BufferStrategy.h"
#include "PF_BufferManger.h"
#include "gtest/gtest.h"
#include <iostream>

namespace{

class PF_BufferStrategyTest : public testing::Test {
protected:
    void SetUp() override{
        LRU_ = new LRU<BufferKey, BufferKeyHash>();
    }

    void TearDown() override{
        delete LRU_;
    } 

protected:
    BufferStrategy<BufferKey>* LRU_;
};

TEST_F(PF_BufferStrategyTest, LRU_test){
    BufferKey b1{1, 2}, b2{234434, 0}, b3{2323, 4546};
    EXPECT_TRUE(LRU_->empty());
    // LRU_->push(b1);
    // LRU_->push(b3);
    // LRU_->push(b2);
    // EXPECT_EQ(LRU_->size(), 3);
}

}