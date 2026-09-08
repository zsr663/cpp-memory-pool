#pragma once
#include <map>
#include <cstddef>

const unsigned MAX_UNIT_SIZE = 1024;

//内存块头部结构体，每个内存单元前面都带这个头部
class MemoryAlloc;
class MemoryBlock
{
public:
    MemoryBlock* pNext = nullptr;
    MemoryAlloc* pAlloc = nullptr;
};

//单个固定大小内存池
class MemoryAlloc
{
public:
    MemoryAlloc(unsigned unitCount_, unsigned unitSize_);
    ~MemoryAlloc();
    void initMemoryAlloc();
    void* allocMem(size_t nSize);
    void freeMem(void* block);

private:
    char* pMemoryAlloc;         //内存池整块内存起始地址
    MemoryBlock* pBlockHeader;  //空闲链表头指针
    unsigned unitCount;         //内存池总块数量
    unsigned unitSize;          //每一块用户可用内存大小
};

//全局内存管理器（单例），管理多个不同规格内存池
class MemoryMgr
{
public:
    static MemoryMgr& getInstance();
    MemoryMgr(const MemoryMgr& memoryMgr) = delete;
    MemoryMgr& operator= (const MemoryMgr& memoryMgr) = delete;
    void* allocMem(size_t nSize);
    void freeMem(void* block);

private:
    MemoryMgr();
    std::map<unsigned, MemoryAlloc*> memoryAllocMap;
    MemoryAlloc memoryAlloc64;
    MemoryAlloc memoryAlloc128;
    MemoryAlloc memoryAlloc256;
    MemoryAlloc memoryAlloc512;
    MemoryAlloc memoryAlloc1024;
};