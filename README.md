# C++ 固定大小内存池（Fixed-size Memory Pool）

一个经典的入门级内存池实现：一次性向系统申请大块内存，之后用**空闲链表（Free List）**做 O(1) 的分配与回收，按 2 的幂分 5 档规格（64/128/256/512/1024 字节）管理，属于简化版的 slab 分配器思想。

## 文件结构

```
├── memoryMgr.h        # 三个核心类声明：MemoryBlock / MemoryAlloc / MemoryMgr
├── memoryMgr.cpp      # 完整实现：初始化、分配、释放、单例
└── 内存池详解.md       # 配套的详细讲解笔记（设计动机、内存布局图、流程演算、潜在问题）
```

## 三层架构

| 类 | 角色 | 职责 |
|----|------|------|
| `MemoryBlock` | 内存块头部 | 每个内存单元前 16 字节（64 位下两个指针）：`pNext` 串空闲链表，`pAlloc` 记录所属池（池外内存为 nullptr） |
| `MemoryAlloc` | 单个固定大小内存池 | 管理一块连续大内存，切分成等大单元串成空闲链表；分配=弹头部，释放=头插 |
| `MemoryMgr` | 全局管理器（Meyers 单例） | 持有 5 个规格的池，按请求大小向上取整到 2 的幂选池；超过 1024 字节或池耗尽时退化为直接 malloc |

## 核心机制

1. **空闲链表**：初始化时把整块内存切成 `unitCount` 个单元，用 `pNext` 串成链表；分配与释放都只操作链表头，O(1)。
2. **头部复用**：链表指针直接存放在空闲块自身的内存里，不额外申请管理结构——free list 的经典技巧。
3. **规格索引**：`map<log2(unitSize), MemoryAlloc*>`，`while ((num = num*2) < nSize)` 找到刚好装得下的档位。
4. **池内/池外统一**：所有内存都带 `MemoryBlock` 头部，释放时靠 `pAlloc` 判断还给池还是直接 `free`，调用方无需区分。

## 使用示例

```cpp
#include "memoryMgr.h"

void* p = MemoryMgr::getInstance().allocMem(100);   // 落入 128 字节池
// ... 使用 p（最多 100 字节）...
MemoryMgr::getInstance().freeMem(p);                // 头插回 128 字节池的空闲链表
```

注意：本实现只管理**裸内存**，不调用构造/析构函数；存放 C++ 对象需配合 placement new 与显式析构。

## 已知局限（详见 内存池详解.md 第 9 节）

- 线程不安全（无锁）
- 不校验 double free 与超规格分配（绕过 MemoryMgr 直接调池的 allocMem 可能溢出）
- 池耗尽后退化的 malloc 内存归还系统而非补进池

## 编译

任意支持 C++11 的编译器直接编译 `memoryMgr.cpp` 即可（无第三方依赖）；配套笔记为 Markdown，可用任意阅读器查看。
