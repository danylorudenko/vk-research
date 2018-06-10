#include <utility>
#include "Application.hpp"

Application::Application(ApplicationDelegate* delegate)
    : delegate_(delegate)
{

}

Application::Application(Application&& rhs)
    : delegate_{ nullptr }
{
    operator=(std::move(rhs));
}

Application& Application::operator=(Application&& rhs)
{
    std::swap(delegate_, rhs.delegate_);
    return *this;
}

Application::~Application()
{

}
