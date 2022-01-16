#include "rm.h"

RM_Record::RM_Record():data_(nullptr), rid_({-1, -1}){}

RC RM_Record::GetData(char* &pData) const{
    pData = data_;
    return RC::SUCCESSS;
}

RC RM_Record::GetRid(RID &rid) const{
    rid = rid_;
    return RC::SUCCESSS;
}