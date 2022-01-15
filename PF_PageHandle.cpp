#include "pf.h"

PF_PageHandle::PF_PageHandle():pageNum_(-1), data_(nullptr){}

RC PF_PageHandle::GetData(char *&pData) const{
    pData = data_;
    return RC::SUCCESSS;
}

RC PF_PageHandle::GetPageNum(PageNum &pageNum) const{
    pageNum = pageNum_;
    return RC::SUCCESSS;
}