#include "ix.h"
#include "pf.h"
#include "rm.h"
#include "sm.h"

#include <gtest/gtest.h>
#include <stdlib.h>

class SM_ManagerTest : public testing::Test {
protected:
    void SetUp()
    {
        pf_manager_ = new PF_Manager();
        rm_manager_ = new RM_Manager(*pf_manager_);
        ix_manager_ = new IX_Manager(*pf_manager_);
        sm_manager_ = new SM_Manager(*ix_manager_, *rm_manager_);
        CUR_DIR_ = new char[255];
        getcwd(CUR_DIR_, 255);
    }

    void TearDown()
    {
        delete CUR_DIR_;
        delete sm_manager_;
        delete ix_manager_;
        delete rm_manager_;
        delete pf_manager_;
    }

    void createDB()
    {
        system(CREATE_DB_CMD_);
    }

    void destoryDB()
    {
        chdir(CUR_DIR_);
        system(DESTORY_DB_CMD_);
    }

protected:
    const char* TEST_DB_DIR_ = "/tmp/testDB";
    char* CUR_DIR_;
    const char* CREATE_DB_CMD_ = "build/bin/dbcreate /tmp/testDB";
    const char* DESTORY_DB_CMD_ = "build/bin/dbdestroy /tmp/testDB";
    PF_Manager* pf_manager_;
    RM_Manager* rm_manager_;
    IX_Manager* ix_manager_;
    SM_Manager* sm_manager_;
};

TEST_F(SM_ManagerTest, SM_CREATE_DB_TEST)
{
    EXPECT_EQ(system(CREATE_DB_CMD_), 0);
    chdir(TEST_DB_DIR_);
    EXPECT_EQ(access(TEST_DB_DIR_, F_OK), 0);
    EXPECT_EQ(access(RELATION_TABLE_NAME, F_OK), 0);
    EXPECT_EQ(access(ATTRIBUTE_TABLE_NAME, F_OK), 0);

    chdir(CUR_DIR_);
    EXPECT_EQ(system(DESTORY_DB_CMD_), 0);
    EXPECT_NE(access(TEST_DB_DIR_, F_OK), 0);
}

TEST_F(SM_ManagerTest, SM_CREATE_DROP_TABLE_TEST)
{
    createDB();
    EXPECT_EQ(sm_manager_->OpenDb(TEST_DB_DIR_), RC::SUCCESS);
    const char* relName = "testRel";
    AttrInfo info[3] {
        { "intTest", AttrType::RD_INT, sizeof(int) },
        { "flotTest", AttrType::RD_FLOAT, sizeof(float) },
        { "strTest", AttrType::RD_STRING, MAXNAME + 1 }
    };
    EXPECT_EQ(sm_manager_->CreateTable(relName, 3, info), RC::SUCCESS);
    EXPECT_EQ(access(relName, F_OK), 0);
    EXPECT_EQ(sm_manager_->DropTable(relName), RC::SUCCESS);
    EXPECT_NE(access(relName, F_OK), 0);

    EXPECT_EQ(sm_manager_->CloseDb(), RC::SUCCESS);
    destoryDB();
}