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
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.CreateFile(name.get()));
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.OpenFile(name.get(), fhd));

    PF_PageHandle p0, p1;
    RETURN_RC_IF_NOT_SUCCESS(fhd.AllocatePage(p0));
    RETURN_RC_IF_NOT_SUCCESS(fhd.AllocatePage(p1));

    // set index head and root page
    setIndexHeader(p0, attrType, attrLength);
    IX_BNodeWapper::init_node(p1);

    PageNum pageNum;
    p0.GetPageNum(pageNum);
    RETURN_RC_IF_NOT_SUCCESS(fhd.MarkDirty(pageNum));
    RETURN_RC_IF_NOT_SUCCESS(fhd.UnpinPage(pageNum));
    p1.GetPageNum(pageNum);
    RETURN_RC_IF_NOT_SUCCESS(fhd.UnpinPage(pageNum));
    RETURN_RC_IF_NOT_SUCCESS(fhd.MarkDirty(pageNum));
    RETURN_RC_IF_NOT_SUCCESS(pf_manager_.CloseFile(fhd));

    return RC::SUCCESSS;
}

std::unique_ptr<char[]> IX_Manager::getFileName(const char* filename, int indexNo)
{
    // file=filename.indexNo
    int len = strlen(filename);
    std::unique_ptr<char[]> name(new char[len + 1 + sizeof(int)]);
    strcpy(name.get(), filename);
    name[len] = '.';
    *((int*)&name[len + 1]) = indexNo;
    return name;
}

void IX_Manager::setIndexHeader(PF_PageHandle& headerPage,
    AttrType attrtype,
    int attrLength)
{
    char* data;
    headerPage.GetData(data);
    auto *hdr = (IX_BFileHeader*) data;
    hdr->attrLength = attrLength;
    hdr->attrType = attrtype;
    hdr->height = 1;
    // sizeof(BNode) = sizeof(int) + order * (attrLength) + (order + 1) * sizeof(rid) < PF_PAGE_SIZE
    hdr->order = (PF_PAGE_SIZE - sizeof(int) - sizeof(RID)) / (attrLength + sizeof(RID));
    hdr->root = {1, 0};
}
