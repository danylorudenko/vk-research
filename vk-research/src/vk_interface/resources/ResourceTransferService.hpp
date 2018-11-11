#pragma once

#include <cstdint>
#include "..\..\class_features\NonCopyable.hpp"
#include "Resource.hpp"

namespace VKW
{

class ImportTable;
class Device;
class Worker;

struct ImageUpdateInfo
{
    void* srcData;
    std::uint32_t dataSize;
    ImageResourceHandle destImage;
    std::uint32_t destImageWidth;
    std::uint32_t destImageHeight;
};


struct ResourceTransferServiceDesc
{
    ImportTable* table_;
    Device* device_;
};

class ResourceTransferService
    : public NonCopyable
{
public:
    ResourceTransferService();
    ResourceTransferService(ResourceTransferServiceDesc const& desc);

    ResourceTransferService(ResourceTransferService&& rhs);
    ResourceTransferService& operator=(ResourceTransferService&& rhs);

    ~ResourceTransferService();

    void UpdateImageWithData(ImageUpdateInfo const& info);


private:
    ImportTable* table_;
    Device* device_;
};

}