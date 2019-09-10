#include "ResourceTransferService.hpp"

#include <utility>

namespace VAL
{

ResourceTransferService::ResourceTransferService()
    : table_{ nullptr }
    , device_{ nullptr }
{

}

ResourceTransferService::ResourceTransferService(ResourceTransferServiceDesc const& desc)
    : table_{ desc.table_ }
    , device_{ desc.device_ }
{

}

ResourceTransferService::ResourceTransferService(ResourceTransferService&& rhs)
    : table_{ nullptr }
    , device_{ nullptr }
{
    operator=(std::move(rhs));
}

ResourceTransferService& ResourceTransferService::operator=(ResourceTransferService&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);

    return *this;
}

ResourceTransferService::~ResourceTransferService()
{

}

void ResourceTransferService::UpdateImageWithData(ImageUpdateInfo const& info)
{

}

}