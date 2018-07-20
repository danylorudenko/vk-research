#include <utility>
#include "ResourcesController.hpp"

namespace VKW
{

ResourcesController::ResourcesController()
{
}

ResourcesController::ResourcesController(ResourcesController&& rhs)
{
    operator=(std::move(rhs));
}

ResourcesController& ResourcesController::operator=(ResourcesController&& rhs)
{
    return *this;
}

ResourcesController::~ResourcesController()
{
}

}