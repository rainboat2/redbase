#include <sstream>

#include "IX_Internal.h"
#include "ix.h"
#include "string.h"

IX_Manager::IX_Manager(PF_Manager& pfm)
    : pf_manager_(pfm)
{
}

RC IX_Manager::CreateIndex(
    const char* filename,
    int indexNo,
    AttrType attrType,
    int attrLength)
{
    auto name = getFileName(filename, indexNo);
    PF_FileHandle fhd;
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.CreateFile(name.c_str()));
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.OpenFile(name.c_str(), fhd));

    PF_PageHandle p0, p1;
    RETURN_RC_IF_NOT_SUCCESS(fhd.AllocatePage(p0));
    RETURN_RC_IF_NOT_SUCCESS(fhd.AllocatePage(p1));

    // set index head and root page
    setIndexHeader(p0, attrType, attrLength);
    IX_BNodeWapper::initNode(p1, attrType, attrLength);

    PageNum pageNum;
    p0.GetPageNum(pageNum);
    RETURN_RC_IF_NOT_SUCCESS(fhd.MarkDirty(pageNum));
    RETURN_RC_IF_NOT_SUCCESS(fhd.UnpinPage(pageNum));
    p1.GetPageNum(pageNum);
    RETURN_RC_IF_NOT_SUCCESS(fhd.MarkDirty(pageNum));
    RETURN_RC_IF_NOT_SUCCESS(fhd.UnpinPage(pageNum));
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.CloseFile(fhd));

    return RC::SUCCESSS;
}

RC IX_Manager::OpenIndex(const char* fileName, int indexNo, IX_IndexHandle& handle)
{
    if (handle.isOpen_)
        return RC::IX_INDEX_OPENED;

    auto name = getFileName(fileName, indexNo);
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.OpenFile(name.c_str(), handle.pf_fileHandle_));

    PF_PageHandle headPage, rootPage;
    char *rootData, *headerData;

    // read header from file
    RETURN_RC_IF_NOT_SUCCESS(handle.pf_fileHandle_.GetThisPage(0, headPage));
    headPage.GetData(headerData);
    memcpy(&handle.fileHeader_, headerData, sizeof(IX_BFileHeader));
    handle.pf_fileHandle_.UnpinPage(0);

    // read root form file
    auto& bfh = handle.fileHeader_;
    PageNum rootPageNum;
    handle.fileHeader_.root.GetPageNum(rootPageNum);
    RETURN_RC_IF_NOT_SUCCESS(handle.pf_fileHandle_.GetThisPage(rootPageNum, rootPage));
    rootPage.GetData(rootData);
    handle.root_ = IX_BNodeWapper(bfh.attrLength, bfh.attrType, rootData, { rootPageNum, 0 });

    handle.isOpen_ = true;
    return RC::SUCCESSS;
}

RC IX_Manager::CloseIndex(IX_IndexHandle& handle)
{
    if (!handle.isOpen_)
        return RC::IX_INDEX_CLOSED;

    if (handle.isHeaderChange_)
        RETURN_RC_IF_NOT_SUCCESS(handle.forceHeader());

    RETURN_RC_IF_NOT_SUCCESS(handle.pf_fileHandle_.UnpinPage(handle.root_.getPageNum()));
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.CloseFile(handle.pf_fileHandle_));
    handle.isOpen_ = false;
    return RC::SUCCESSS;
}

RC IX_Manager::DestroyIndex(const char* filename, int indexNo)
{
    auto name = getFileName(filename, indexNo);
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.DestroyFile(name.c_str()));
    return RC::SUCCESSS;
}

std::string IX_Manager::getFileName(const char* filename, int indexNo)
{
    std::stringstream ss;
    ss << filename << '.' << indexNo;
    return ss.str();
}

void IX_Manager::setIndexHeader(PF_PageHandle& headerPage,
    AttrType attrtype,
    int attrLength)
{
    char* data;
    headerPage.GetData(data);
    auto* hdr = (IX_BFileHeader*)data;
    hdr->attrLength = attrLength;
    hdr->attrType = attrtype;
    hdr->pageNums = 2;
    hdr->height = 1;
    hdr->order = IX_BNodeWapper::countOrder(attrLength);
    hdr->root = { 1, 0 };
}
