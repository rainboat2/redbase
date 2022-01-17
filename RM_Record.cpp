#include "rm.h"

RM_Record::RM_Record():data_(nullptr), rid_({-1, -1}){}

RM_Record::RM_Record(RM_Record &&record){
    data_ = record.data_;
    rid_ = record.rid_;
    record.data_ == nullptr;
}

RM_Record::~RM_Record(){
    if (data_ != nullptr)
        delete[] data_;
}

RC RM_Record::GetData(char* &pData) const{
    pData = data_;
    return RC::SUCCESSS;
}

RC RM_Record::GetRid(RID &rid) const{
    rid = rid_;
    return RC::SUCCESSS;
}

void RM_Record::setData(char *buf, int size){
    if (data_ != nullptr)
        delete[] data_;
    data_ = new char[size];
    memcpy(data_, buf, size);
}