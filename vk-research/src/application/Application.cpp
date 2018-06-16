#include <utility>
#include <cassert>

#include <Windows.h>

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

void Application::run()
{
    assert(delegate_);

    delegate_->start();

    MSG message{};
    message.message = WM_NULL;

    while (message.message != WM_QUIT) {
        if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        else {
            delegate_->update();
        }
    }

    delegate_->shutdown();
}
