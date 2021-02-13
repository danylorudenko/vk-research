#include <memory\Memory.hpp>

DRE_BEGIN_NAMESPACE


U64 constexpr DATA_EXCHANGE_ARENA_SIZE  = DataExchangeAllocatorBuddy::RequiredMemorySize();
U64 constexpr PERSISTENT_ARENA_SIZE     = 1024 * 1024;
U64 constexpr THREAD_LOCAL_ARENA_SIZE   = 1024 * 1024 * 16;




thread_local void*               g_ThreadLocalArena = nullptr;
thread_local AllocatorScopeStack g_ThreadLocalStackAllocator;


void* g_PersistentArena     = nullptr;
void* g_DataExchangeArena   = nullptr;

AllocatorLinear                 g_PersistentDataAllocator;
DataExchangeAllocatorBuddy      g_DataExchangeAllocator;





void InitializeGlobalMemory()
{
    g_PersistentArena = DRE_MALLOC(PERSISTENT_ARENA_SIZE);
    g_PersistentDataAllocator = AllocatorLinear(g_PersistentArena, PERSISTENT_ARENA_SIZE);


    g_DataExchangeArena = DRE_MALLOC(DATA_EXCHANGE_ARENA_SIZE);
    g_DataExchangeAllocator = DataExchangeAllocatorBuddy(g_DataExchangeArena, DATA_EXCHANGE_ARENA_SIZE);
}

void TerminateGlobalMemory()
{
    DRE_FREE(g_DataExchangeArena);
    DRE_FREE(g_PersistentArena);
}

void InitializeThreadLocalMemory()
{
    g_ThreadLocalArena = DRE_MALLOC(THREAD_LOCAL_ARENA_SIZE);
    g_ThreadLocalStackAllocator = AllocatorScopeStack(g_ThreadLocalArena, THREAD_LOCAL_ARENA_SIZE);
}

void TerminateThreadLocalMemory()
{
    DRE_FREE(g_ThreadLocalArena);
}

DRE_END_NAMESPACE
