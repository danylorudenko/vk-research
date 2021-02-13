#pragma once

#include <memory\AllocatorLinear.hpp>
#include <memory\AllocatorScopeStack.hpp>
#include <memory\AllocatorBuddy.hpp>


#define DRE_MALLOC(size) std::malloc(size)
#define DRE_FREE(memory) std::free(memory)


DRE_BEGIN_NAMESPACE


/*
*
* Root application memory control header file
* Defining allocation contexts
*
*/


void InitializeGlobalMemory();
void TerminateGlobalMemory();

void InitializeThreadLocalMemory();
void TerminateThreadLocalMemory();


// To be used with most thread-local work
extern thread_local AllocatorScopeStack g_ThreadLocalScratchAllocator;

// To be used with all persistent stuff. WARNING, REQUIRES MANUAL DESTRUCTION
extern AllocatorLinear     g_PersistentDataAllocator;


// To be used for persistent data exchange between threads on JOIN stages, etc.
U64 constexpr DATA_EXCHANGE_LEAF_SIZE   = 1024;
U64 constexpr DATA_EXCHANGE_MAX_DEPTH   = 5;
using  DataExchangeAllocatorBuddy       = AllocatorBuddy<DATA_EXCHANGE_LEAF_SIZE, DATA_EXCHANGE_MAX_DEPTH>;
extern DataExchangeAllocatorBuddy       g_MultithreadDataExchangeAllocator;


DRE_END_NAMESPACE
