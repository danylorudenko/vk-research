#pragma once

#include "../../class_features/NonCopyable.hpp"

namespace VKW
{

class ImportTable;
class Device;

struct ResourceBindingServiceDesc
{
    ImportTable* table_;
    Device* device_;
};

class ResourceBindingService
{
public:
    ResourceBindingService();
    ResourceBindingService(ResourceBindingServiceDesc const& desc);

    ResourceBindingService(ResourceBindingService&& rhs);
    ResourceBindingService& operator=(ResourceBindingService&& rhs);

    ~ResourceBindingService();

private:
    ImportTable* table_;
    Device* device_;

};

}