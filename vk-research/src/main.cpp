#include "application\VulkanApplicationDelegate.hpp"

int main()
{
    HINSTANCE instance = GetModuleHandle(nullptr);
    
    auto* appDelegate = new VulkanApplicationDelegate{ instance, "Vulkan Application", 800, 600, true };
    auto* application = new Application{ appDelegate };

    application->run();

    delete application;
    delete appDelegate;

    return 0;
}