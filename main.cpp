#include "pf.h"
#include "PF_BufferManger.h"
#include "PF_BufferStrategy.h"

#include <unordered_map>
#include <iostream>

// struct BufferBlockDesc
// {
//     int fd;
//     PageNum pageNum;
//     char* data;
//     short int pinCount;
//     bool isDirty;

    
// };

int main(){
    BufferStrategy<BufferKey>* LRU_ = new LRU<BufferKey, BufferKeyHash>();
    BufferKey b1{1, 2}, b2{234434, 0}, b3{2323, 4546};
    LRU_->push(b1);
    delete LRU_;
    return 0;
}