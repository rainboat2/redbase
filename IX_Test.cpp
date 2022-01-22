#include <array>
#include <gtest/gtest.h>
#include <iostream>

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
    std::array<int, 4> arr { 1, 2, 4, 8 };
    *((int*)buffer) = arr.size();
    int* attr = (int*)buffer + 1;
    for (int i = 0; i < arr.size(); i++) {
        attr[i] = arr[i];
    }
    IX_BNodeWapper node(sizeof(int), AttrType::RD_INT, buffer, { -1, -1 });
    EXPECT_EQ(node.size(), arr.size());

    const std::map<int, int> indexOf = { { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 1 }, { 3, 2 }, { 4, 2 }, { 5, 3 }, { 8, 3 }, { 12, 4 } };
    for (auto pair : indexOf){
        EXPECT_EQ(node.indexOf(&pair.first), pair.second);
    }
}