#include "ix.h"

IX_IndexHandle::IX_IndexHandle()
    : isOpen_(false)
{
}

RC IX_IndexHandle::InsertEntry(void* pData, const RID& rid)
{
    return RC::SUCCESSS;
}

IX_BInsertUpEntry IX_IndexHandle::InsertEntry(IX_BNodeWapper& cur, void* pData, const RID& rid, int level)
{
    assert(level <= fileHeader_.height && level >= -1);

    IX_BInsertUpEntry curEntry;
    if (level == -1) {
        auto upEntry = InsertEntry(root_, pData, rid, level + 1);
        if (upEntry.isSpilt)
            changeRoot(upEntry);
    } else if (level < fileHeader_.height) {
        // not leaf node
        int index = cur.indexOf(pData);
        IX_BNodeWapper next = readBNodeFrom(cur.getRid(index));
        auto upEntry = InsertEntry(next, pData, rid, level + 1);
        if (upEntry.isSpilt) {
            if (cur.isFull()) {
                IX_BNodeWapper newNode = createBNode();
                curEntry = cur.notLeafSpiltAndInsert(upEntry, newNode);
                pf_fileHandle_.MarkDirty(newNode.getPageNum());
                pf_fileHandle_.UnpinPage(newNode.getPageNum());
            } else {
                cur.notLeafInsert(upEntry);
            }
            pf_fileHandle_.MarkDirty(next.getPageNum());
        }
        pf_fileHandle_.UnpinPage(next.getPageNum());
    } else {
        // leaf node
        if (cur.isFull()) {
            IX_BNodeWapper newNode = createBNode();
            curEntry = cur.leafSpiltAndInsert(pData, rid, newNode);
            pf_fileHandle_.MarkDirty(newNode.getPageNum());
            pf_fileHandle_.UnpinPage(newNode.getPageNum());
        } else {
            cur.leafInsert(pData, rid);
        }
    }
    return curEntry;
}

IX_BNodeWapper IX_IndexHandle::readBNodeFrom(const RID& rid)
{
    PageNum pageNum;
    rid.GetPageNum(pageNum);
    PF_PageHandle page;
    pf_fileHandle_.GetThisPage(pageNum, page);
    char* data;
    page.GetData(data);
    return IX_BNodeWapper(fileHeader_.attrLength, fileHeader_.attrType, data, rid);
}

IX_BNodeWapper IX_IndexHandle::createBNode()
{
    PF_PageHandle page;
    RC rc = pf_fileHandle_.AllocatePage(page);
    assert(rc == RC::SUCCESSS);
    IX_BNodeWapper::initNode(page);
    PageNum pageNum;
    char* data;
    page.GetData(data);
    page.GetPageNum(pageNum);
    return IX_BNodeWapper(fileHeader_.attrLength, fileHeader_.attrType, data, { pageNum, 0 });
}