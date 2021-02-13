#pragma once

#include <foundation\Common.hpp>
#include <math\SimpleMath.hpp>
#include <memory\Pointer.hpp>
#include <memory\MemoryOps.hpp>

DRE_BEGIN_NAMESPACE


/*
*
* Buddy allocation implementation.
*
* Basic interface:
*
*   + Alloc     (size)
*   + Free      ()
*   + MemorySize()
*   + Reset     ()
*
*   + RequiredMemorySize() <-- this returns required memory size on compile time
*
*
*
*
* Algorithm:
*
*  1  get_target_depth()
*  2  if(no free chunk on target_depth)
*  3      go to higher depth
*  4      try split chunk higher
*  5      go to 2
*  6  else
*  7      get free chunk on target_depth
*
*
* Runtime scheme:
*                                 _______________________________
*   *freelist_ptr[0]-->null      |ssssssssssssss_0_ssssssssssssss|
*   *freelist_ptr[1]------------>|_______1_______|ssssss_2_ssssss| <- split
*   *freelist_ptr[2]-----        |___3___|___4___|XX_5_XX|___6___|
*                       |                                    ^
*                       |____________________________________|
*
* Here user requrested small chunk.
* Initially, only the biggest block is considered free.
* 1 - try get chunk on lowest depth
* 2 - there is no block, try get free chunk higher and split it
* 3 - there is no block, try to get free chunk on highest level, there is a root chunk, split it and return recursively
*
*/
template<U64 LEAF_SIZE, U8 MAX_DEPTH>
class AllocatorBuddy
{
public:
    inline void* Alloc(U64 size)
    {
        DRE_ASSERT(size < DRE_U32_MAX, "AllocatorBuddy currently supports allocations only < DRE_U32_MAX");

        U64 const potSize = (U64)NextPowOf2((U32)size);
        U8 const depth = GetDepthBySize(potSize);
        void* result = RecursiveAllocInternal(potSize, depth);
        if (result != nullptr)
        {
            MetaSetChunkDepth(result, depth);
            U32 const globalIndex = GetGlobalStateIndex(result, depth);
            MetaSetChunkUsedByGlobalIndex(globalIndex);
        }

        return result;
    }

    inline void Free(void* memory)
    {
        U8 const depth = MetaGetChunkDepth(memory);

        U32 const globalIndex = GetGlobalStateIndex(memory, depth);
        MetaSetChunkFreeByGlobalIndex(globalIndex);

        MetaPutFreeChunkOnDepth(depth, memory);
        RecursiveMergeInternal(depth, memory);
    }

private:
    void* RecursiveAllocInternal(U64 size, U8 depth)
    {
        void* freeChunk = nullptr;
        if (freeChunk = MetaExtractFirstFreeChunkOnDepth(depth))
        {
            return freeChunk;
        }
        else
        {
            if (depth == 0)
                return nullptr;

            freeChunk = RecursiveAllocInternal(size, depth - 1);
            if (freeChunk) 
            {
                // here we split chunk on *depth*, put two children on free list on (depth)
                SplitChunkOnDepth(freeChunk, depth - 1);
            }
            else
            {
                return nullptr;
            }
        }

        return MetaExtractFirstFreeChunkOnDepth(depth);
    }

    void SplitChunkOnDepth(void* chunk, U8 depth)
    {
        U64 const childSize = ChunkSizeByDepth(depth) / 2;
        void* left = chunk;
        void* right = PtrAdd(chunk, (PtrDiff)childSize);

        U8 const childDepth = depth + 1;
        MetaSetChunkDepth(left, childDepth);
        MetaSetChunkDepth(right, childDepth);

        U32 const globalId = GetGlobalStateIndex(chunk, depth);
        MetaSetChunkUsedByGlobalIndex(globalId);

        MetaPutFreeChunkOnDepth(childDepth, left);
        MetaPutFreeChunkOnDepth(childDepth, right);
    }


    void RecursiveMergeInternal(U8 depth, void* memory)
    {
        if(depth == 0)
            return;

        U32 const index = GetIndexInDepth(memory, depth);
        U64 const size = GetChunkSize(memory);
        if (index % 2 == 0) // uneven
        {
            // buddy is on the right
            void* buddy = PtrAdd(memory, size);
            if (MetaIsChunkFreeOnDepth(buddy, depth)) // merge
            {
                MetaRemoveFreeChunkOnDepth(depth, memory);
                MetaRemoveFreeChunkOnDepth(depth, buddy);

                MetaPutFreeChunkOnDepth(depth - 1, memory);

                MetaSetChunkDepth(memory, depth - 1);
                U32 const globalIndex = GetGlobalStateIndex(memory, depth - 1);
                MetaSetChunkFreeByGlobalIndex(globalIndex);

                RecursiveMergeInternal(depth - 1, memory);
            }
        }
        else // even
        {
            // buddy is on the left
            void* buddy = PtrAdd(memory, -((PtrDiff)size));
            if (MetaIsChunkFreeOnDepth(buddy, depth))
            {
                MetaRemoveFreeChunkOnDepth(depth, memory);
                MetaRemoveFreeChunkOnDepth(depth, buddy);

                MetaPutFreeChunkOnDepth(depth - 1, buddy);
                
                MetaSetChunkDepth(buddy, depth - 1);
                U32 const globalIndex = GetGlobalStateIndex(buddy, depth - 1);
                MetaSetChunkFreeByGlobalIndex(globalIndex);

                RecursiveMergeInternal(depth - 1, buddy);
            }
        }
    }


public:
    inline U64 MemorySize() const
    {
        return m_Size;
    }

    inline void Reset()
    {
        DreMemZero(m_Memory, m_Size);
        m_ChunksStart = ChunksStart();

        MetaPutFreeChunkOnDepth(0, m_ChunksStart);
        for (U32 i = 1; i < (U32)(MaxDepth() + 1); ++i)
        {
            m_MetaData->depthFreeLists[i] = nullptr;
        }

        MetaSetChunkDepth(m_ChunksStart, 0);
    }

    inline static constexpr U64 RequiredMemorySize()
    {
        return RootChunkSize() + RootChunkAlignment() + sizeof(MetaData);
    }



public:
    AllocatorBuddy()
        : m_Memory      { nullptr }
        , m_Size        { 0 }
        , m_MetaData    { nullptr }
        , m_ChunksStart { nullptr }
    {
    }

    AllocatorBuddy(void* memory, U64 size)
        : m_Memory      { memory }
        , m_Size        { size }
        , m_MetaData    { (MetaData*)memory }
        , m_ChunksStart { nullptr }
    {
        DRE_ASSERT(m_Memory != nullptr, "AlloocatorRandom: received null memory.");
        DRE_ASSERT(m_Size != 0, "AllocatorBuddy: received null size.");

        Reset();
    }

    AllocatorBuddy(AllocatorBuddy&& rhs)
        : m_Memory      { nullptr }
        , m_Size        { 0 }
        , m_MetaData    { nullptr }
        , m_ChunksStart { nullptr }
    {
        operator=(DRE_MOVE(rhs));
    }

    AllocatorBuddy& operator=(AllocatorBuddy&& rhs)
    {
        m_Memory = rhs.m_Memory;            rhs.m_Memory = nullptr;
        m_Size = rhs.m_Size;                rhs.m_Size = 0;
        m_MetaData = rhs.m_MetaData;        rhs.m_MetaData = nullptr;
        m_ChunksStart = rhs.m_ChunksStart;  rhs.m_ChunksStart = nullptr;

        return *this;
    }

    AllocatorBuddy(AllocatorBuddy const&) = delete;
    AllocatorBuddy& operator=(AllocatorBuddy const& rhs) = delete;

    ~AllocatorBuddy()
    {
        m_Memory = nullptr;
        m_Size = 0;
        m_MetaData = nullptr;
        m_ChunksStart = nullptr;
    }



private:
    // Generic allocator section
    void*       m_Memory;
    U64         m_Size;




// chunk management
private:
    static constexpr bool IsValidRootChunkSize()
    {
        return IsPowOf2(RootChunkSize());
    }

    static constexpr U8 MaxDepth()
    {
        return MAX_DEPTH;
    }

    static constexpr U32 LeafSize()
    {
        return LEAF_SIZE;
    }

    static constexpr U32 LeavesCount()
    {
        return 1U << MAX_DEPTH;
    }

    static constexpr U32 AllPossibleChunksCount()
    {
        return 1U << (MAX_DEPTH + 1); // + -1 maybe also ?
    }

    static constexpr U64 RootChunkSize()
    {
        return (LeavesCount()) * LEAF_SIZE;
    }

    static constexpr U32 RootChunkAlignment()
    {
        return 64;
    }

    static constexpr U64 ChunkSizeByDepth(U8 depth)
    {
        return RootChunkSize() >> depth;
    }

    static constexpr U8 GetDepthBySize(U64 size)
    {
        DRE_ASSERT(IsPowOf2(size), "Argument must be PowOf2.");

        U64 constexpr rootPow = Log2(RootChunkSize());
        U64 const pow = Log2(size);

        return (U8)(rootPow - pow);
    }

    static constexpr U64 GetChunksCountOnDepth(U8 depth)
    {
        return RootChunkSize() / ChunkSizeByDepth(depth);
    }

    // returns index on which states of the chunks on *depth* starts
    static constexpr U32 GetChunkStateDepthStart(U8 depth)
    {
        return (1U << depth) - 1;
    }

    inline void* ChunksStart()
    {
        return PtrAlign(PtrAdd(m_Memory, sizeof(MetaData)), RootChunkAlignment());
    }

    inline U64 GetChunkSize(void* chunk)
    {
        return ChunkSizeByDepth(MetaGetChunkDepth(chunk));
    }

    inline U32 GetIndexInDepth(void* chunk, U8 depth)
    {
        return (U32)(PtrDifference(chunk, m_ChunksStart) / ChunkSizeByDepth(depth));
    }

    inline U32 GetGlobalStateIndex(void* chunk, U8 depth)
    {
        U32 const depthGlobalIndex = GetChunkStateDepthStart(depth);
        U32 const indexInDepth = GetIndexInDepth(chunk, depth);
        return depthGlobalIndex + indexInDepth;
    }

    static_assert(IsValidRootChunkSize(), "AllocatorBuddy: root chunk is not POT.");


    
// metadata
private:
    inline bool MetaIsChunkFreeOnDepth(void* chunk, U8 depth)
    {
        U32 const stateGlobalIndex = GetGlobalStateIndex(chunk, depth);
        return MetaIsChunkFreeByGlobalIndex(stateGlobalIndex);
    }
    
    inline bool MetaIsChunkFreeByGlobalIndex(U32 globalIndex)
    {
        return ((m_MetaData->chunksStates[globalIndex / 8])  &  (1  <<  (globalIndex % 8))) == 0;
    }
    
    inline void MetaSetChunkUsedByGlobalIndex(U32 globalIndex)
    {
        m_MetaData->chunksStates[globalIndex / 8]  |=  (1 << (globalIndex % 8));
    }

    inline void MetaSetChunkFreeByGlobalIndex(U32 globalIndex)
    {
        m_MetaData->chunksStates[globalIndex / 8]  &=  (~(1 << (globalIndex % 8)));
    }

    inline U8 MetaGetChunkDepth(void* chunk)
    {
                                       // lowest leaf index
        return m_MetaData->chunksDepth[PtrDifference(chunk, m_ChunksStart) / LeafSize()];
    }

    inline void MetaSetChunkDepth(void* chunk, U8 depth)
    {
                                // lowest leaf index
        m_MetaData->chunksDepth[PtrDifference(chunk, m_ChunksStart) / LeafSize()] = depth;
    }

    inline void* MetaExtractFirstFreeChunkOnDepth(U8 depth)
    {
        if (m_MetaData->depthFreeLists[depth] != nullptr)
        {
            MetaChunkHeader* result = m_MetaData->depthFreeLists[depth];
            if (result->next != nullptr)
            {
                result->next->prev = nullptr; // no prev because it's first
            }
            m_MetaData->depthFreeLists[depth] = result->next;
            return result;
        }
        else
        {
            return nullptr;
        }

    }

    inline void MetaRemoveFreeChunkOnDepth(U8 depth, void* chunk)
    {
        if (chunk == m_MetaData->depthFreeLists[depth])
        {
            m_MetaData->depthFreeLists[depth] = m_MetaData->depthFreeLists[depth]->next;
            return;
        }

        MetaChunkHeader* header = (MetaChunkHeader*)chunk;
        MetaChunkHeader* prev = header->prev;
        MetaChunkHeader* next = header->next;

        prev->next = next;

        if (next)
            next->prev = prev;
    }

    inline void MetaPutFreeChunkOnDepth(U8 depth, void* chunk)
    {
        MetaChunkHeader* lastFree = m_MetaData->depthFreeLists[depth];
        MetaChunkHeader* newFree = (MetaChunkHeader*)chunk;
        if (lastFree)
        {
            lastFree->next = newFree;

            newFree->prev = lastFree;
            newFree->next = nullptr;
        }
        else
        {
            newFree->prev = nullptr;
            newFree->next = nullptr;

            m_MetaData->depthFreeLists[depth] = newFree;
        }
    }

    struct MetaChunkHeader
    {
        MetaChunkHeader* prev;
        MetaChunkHeader* next;
    };

    struct MetaData
    {
        // freelists for chunks on all depths
        MetaChunkHeader* depthFreeLists[MaxDepth() + 1];
        
        // in 1 bits we will store states of the chunks on all levels: Allocated/Free
        // 0 - free, 1 - used
        U8 chunksStates[AllPossibleChunksCount() / 8];

        // for all possible locations buddy can return
        U8 chunksDepth[LeavesCount()];
    };

    MetaData*   m_MetaData;
    void*       m_ChunksStart;
};

DRE_END_NAMESPACE

