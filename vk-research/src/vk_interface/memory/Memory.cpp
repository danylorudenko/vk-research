#include "Memory.hpp"

#include <utility>

#include <vk_interface/memory/MemoryController.hpp>

namespace VKW
{

MemoryPageHandle::MemoryPageHandle()
    : page_{ nullptr }
{
}

MemoryPageHandle::MemoryPageHandle(MemoryPage* page)
    : page_{ page }
{
}

MemoryPageHandle::MemoryPageHandle(MemoryPageHandle const& rhs)
    : page_{ nullptr }
{
    operator=(rhs);
}

MemoryPageHandle::MemoryPageHandle(MemoryPageHandle&& rhs)
    : page_{ nullptr }
{
    operator=(std::move(rhs));
}

MemoryPageHandle& MemoryPageHandle::operator=(MemoryPageHandle const& rhs) = default;

MemoryPageHandle& MemoryPageHandle::operator=(MemoryPageHandle&& rhs) = default;

MemoryPageHandle::~MemoryPageHandle() = default;

MemoryPage* MemoryPageHandle::GetPage() const
{
    return page_;
}



}

