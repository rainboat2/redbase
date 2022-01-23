#include "ix.h"

IX_IndexHandle::IX_IndexHandle()
    : isOpen_(false)
    , isHeaderChange_(false)
{
}

IX_IndexHandle::~IX_IndexHandle()
{
}

RC IX_IndexHandle::InsertEntry(void* pData, const RID& rid)
{
    IX_BInsertUpEntry up = InsertEntry(root_, pData, rid, 0);
    if (up.isSpilt)
        changeRoot(up);
    return RC::SUCCESSS;
}

void IX_IndexHandle::changeRoot(IX_BInsertUpEntry& entry)
{
    IX_BNodeWapper newRoot = createBNode();
    newRoot.setAttr(0, entry.attr);
    newRoot.setRid(0, { root_.getPageNum(), 0 });
    newRoot.setRid(1, entry.right);
    pf_fileHandle_.UnpinPage(root_.getPageNum());
    fileHeader_.height += 1;
    fileHeader_.root = { newRoot.getPageNum(), 0 };
    isHeaderChange_ = true;
    root_ = newRoot;
}

IX_BInsertUpEntry IX_IndexHandle::InsertEntry(IX_BNodeWapper& cur, void* pData, const RID& rid, int level)
{
    assert(level <= fileHeader_.height && level >= 0);

    IX_BInsertUpEntry curEntry;
    if (level < fileHeader_.height) {
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
    
    fileHeader_.pageNums ++;
    isHeaderChange_ = true;
    return IX_BNodeWapper(fileHeader_.attrLength, fileHeader_.attrType, data, { pageNum, 0 });
}

RC IX_IndexHandle::forceHeader()
{
    if (!isHeaderChange_)
        return RC::SUCCESSS;
    PF_PageHandle page;
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.GetThisPage(0, page));
    char* data;
    page.GetData(data);
    memcpy(data, &fileHeader_, sizeof(IX_BFileHeader));
    isHeaderChange_ = false;
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.MarkDirty(0));
    RETURN_RC_IF_NOT_SUCCESS(pf_fileHandle_.UnpinPage(0));
    return RC::SUCCESSS;
}