#ifndef PF_HH
#define PF_HH

#include "redbase.h"
#include "PF_BufferManager.h"
#include "PF_Internal.h"

using PageNum = int;
#define ALL_PAGES -1
#define PF_PAGE_SIZE 4096 - sizeof(int)


class PF_FileHandle;
class PF_Manager;

class PF_PageHandle {
public:
    PF_PageHandle  ();                         
    ~PF_PageHandle () = default;
    PF_PageHandle  (const PF_PageHandle &pageHandle) = default;
                           
    PF_PageHandle& operator= (const PF_PageHandle &pageHandle) = default;

    RC GetData        (char *&pData) const;    
    RC GetPageNum     (PageNum &pageNum) const;
    friend class PF_FileHandle;
private:
  PageNum pageNum_;
  char *data_;
};


class PF_FileHandle {
public:
    PF_FileHandle  ();
    ~PF_FileHandle ();
    PF_FileHandle  (const PF_FileHandle &fileHandle) = delete;
    PF_FileHandle& operator= (const PF_FileHandle &fileHandle) = delete;

    RC GetFirstPage   (PF_PageHandle &pageHandle) const;
    RC GetLastPage    (PF_PageHandle &pageHandle) const;   // Get the last page

    RC GetNextPage    (PageNum current, PF_PageHandle &pageHandle) const; 

    RC GetPrevPage    (PageNum current, PF_PageHandle &pageHandle) const;

    RC GetThisPage    (PageNum pageNum, PF_PageHandle &pageHandle) const;  

    RC AllocatePage   (PF_PageHandle &pageHandle);         // Allocate a new page
    RC DisposePage    (PageNum pageNum);                   // Dispose of a page 
    RC MarkDirty      (PageNum pageNum) const;             // Mark a page as dirty
    RC UnpinPage      (PageNum pageNum) const;             // Unpin a page
    RC ForcePages     (PageNum pageNum = ALL_PAGES) const;             // Write dirty page(s) to disk
    friend class PF_Manager;

private:
    RC appendFileBlockToEnd();
    RC ForceHeader();

private:
    const char *filename_;
    int fd_;
    bool isOpen_;
    bool isHeadChange_;
    PF_BufferManager *bufferManager_;
    PF_FileHeader header_;
};

class PF_Manager
{
public:
    PF_Manager    ();                              // Constructor
    ~PF_Manager   ();                              // Destructor
    PF_Manager(PF_Manager &manager) = delete;
    PF_Manager& operator=(const PF_Manager &manager) = delete;
    RC CreateFile    (const char *fileName);       // Create a new file
    RC DestroyFile   (const char *fileName);       // Destroy a file
    RC OpenFile      (const char *fileName, PF_FileHandle &fileHandle);  
                                                   // Open a file
    RC CloseFile     (PF_FileHandle &fileHandle);  // Close a file
    RC AllocateBlock (char *&buffer);              // Allocate a new scratch page in buffer
    RC DisposeBlock  (char *buffer);               // Dispose of a scratch page

private:
    PF_BufferManager *buffer_manger_;
};

#endif