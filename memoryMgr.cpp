#include "memoryMgr.h"
#include <iostream>
#include <assert.h>
#include <cstdlib>

//MemoryAlloc构造函数
MemoryAlloc::MemoryAlloc(unsigned unitCount_, unsigned unitSize_)
    : unitCount(unitCount_), unitSize(unitSize_),
    pMemoryAlloc(nullptr), pBlockHeader(nullptr)
{

}

//MemoryAlloc析构，释放整块大内存
MemoryAlloc::~MemoryAlloc()
{
    if (pMemoryAlloc)
        free(pMemoryAlloc);
}

//初始化内存池：malloc一整块连续内存，把所有块串成空闲链表
void MemoryAlloc::initMemoryAlloc()
{
    assert(pMemoryAlloc == nullptr);
    pMemoryAlloc = (char*)malloc(unitCount * (unitSize + sizeof(MemoryBlock)));
    if (!pMemoryAlloc)
    {
        std::cout << "MemoryAlloc::initMemoryAlloc malloc fail" << std::endl;
        return;
    }
    pBlockHeader = (MemoryBlock*)pMemoryAlloc;
    for (unsigned i = 0; i < unitCount; ++i)
    {
        MemoryBlock* pMemoryBlock = (MemoryBlock*)(pMemoryAlloc +
            i * (unitSize + sizeof(MemoryBlock)));
        pMemoryBlock->pAlloc = this;
        if (i != unitCount - 1)
        {
            pMemoryBlock->pNext = (MemoryBlock*)(pMemoryAlloc +
                (i + 1) * (unitSize + sizeof(MemoryBlock)));
        }
        else
        {
            pMemoryBlock->pNext = nullptr;
        }
    }
}

//内存池分配内存
void* MemoryAlloc::allocMem(size_t nSize)
{
    if (!pBlockHeader)
    {
        //内存池内部空闲块耗尽，直接malloc大块（不属于内存池管理）
        MemoryBlock* pMemoryBlock = (MemoryBlock*)((char*)malloc(nSize
            + sizeof(MemoryBlock)));
        pMemoryBlock->pAlloc = nullptr;
        pMemoryBlock->pNext = nullptr;
        return (char*)pMemoryBlock + sizeof(MemoryBlock);
    }
    else
    {
        //取空闲链表头部一块返回
        char* pReturn = ((char*)pBlockHeader + sizeof(MemoryBlock));
        pBlockHeader = pBlockHeader->pNext;
        return pReturn;
    }
}

//归还内存到内存池空闲链表头部
void MemoryAlloc::freeMem(void* block)
{
    MemoryBlock* pMemoryBlock = (MemoryBlock*)((char*)block - sizeof(MemoryBlock));
    pMemoryBlock->pNext = pBlockHeader;
    pBlockHeader = pMemoryBlock;
}

//MemoryMgr单例获取
MemoryMgr& MemoryMgr::getInstance()
{
    static MemoryMgr instance;
    return instance;
}

//MemoryMgr构造函数，初始化5个内存池，装入map
MemoryMgr::MemoryMgr()
    :memoryAlloc64(10, 64),
    memoryAlloc128(10, 128),
    memoryAlloc256(10, 256),
    memoryAlloc512(10, 512),
    memoryAlloc1024(10, 1024)
{
    memoryAlloc64.initMemoryAlloc();
    memoryAlloc128.initMemoryAlloc();
    memoryAlloc256.initMemoryAlloc();
    memoryAlloc512.initMemoryAlloc();
    memoryAlloc1024.initMemoryAlloc();

    memoryAllocMap.insert(std::pair<unsigned, MemoryAlloc*>(6, &memoryAlloc64));
    memoryAllocMap.insert(std::pair<unsigned, MemoryAlloc*>(7, &memoryAlloc128));
    memoryAllocMap.insert(std::pair<unsigned, MemoryAlloc*>(8, &memoryAlloc256));
    memoryAllocMap.insert(std::pair<unsigned, MemoryAlloc*>(9, &memoryAlloc512));
    memoryAllocMap.insert(std::pair<unsigned, MemoryAlloc*>(10, &memoryAlloc1024));
}

//全局内存管理器分配
void* MemoryMgr::allocMem(size_t nSize)
{
    if (nSize <= MAX_UNIT_SIZE)
    {
        unsigned num = 32;
        unsigned i = 6;
        while ((num = num * 2) < nSize)
        {
            ++i;
        }
        return memoryAllocMap[i]->allocMem(nSize);
    }
    else
    {
        //大于1024，直接malloc，不走内存池
        MemoryBlock* pMemoryBlock = (MemoryBlock*)((char*)malloc(nSize + sizeof(MemoryBlock)));
        if (!pMemoryBlock)
        {
            return nullptr;
        }
        pMemoryBlock->pAlloc = nullptr;
        pMemoryBlock->pNext = nullptr;
        return (char*)pMemoryBlock + sizeof(MemoryBlock);
    }
}

//全局内存管理器释放内存
void MemoryMgr::freeMem(void* block)
{
    MemoryBlock* pMemoryBlock = (MemoryBlock*)((char*)block - sizeof(MemoryBlock));
    if (!pMemoryBlock->pAlloc)
    {
        //不属于内存池，直接free
        free(pMemoryBlock);
    }
    else
    {
        //还给对应的内存池
        pMemoryBlock->pAlloc->freeMem(block);
    }
}