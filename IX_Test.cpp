#include <gtest/gtest.h>
#include <iostream>

#include "pf.h"
#include "rm.h"
#include "ix.h"

class IX_BNodeWapperTest : public testing::Test {
protected:
    void SetUp() override { }
    void TearDown() override { }
};

TEST_F(IX_BNodeWapperTest, INDEX_OF_TEST){
    char buffer[PF_PAGE_SIZE];
    const unsigned int nodeSize = 23;
    *((int *) buffer) = nodeSize;
    int *arr = (int*)buffer + 1;
    for (int i = 0; i < nodeSize; i++){
        arr[i] = i * 2;
    }
    IX_BNodeWapper node(sizeof(int), AttrType::RD_INT, buffer);
    for (int i = 0; i < nodeSize; i++){
        EXPECT_EQ(i * 2, *(int*)node.getData(i));
    }
    EXPECT_EQ(nodeSize, node.size());
    for (int i = 0; i < nodeSize * 2; i++){
        int index = node.indexOf(&i);
        // std::cout << "i = " << i << ", indexof i = " << index << std::endl;
        EXPECT_EQ(index, i / 2);
    }
}