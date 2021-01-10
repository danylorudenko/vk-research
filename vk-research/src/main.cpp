#include <application\VulkanApplicationDelegate.hpp>

int main()
{
    HINSTANCE instance = GetModuleHandle(nullptr);
    
    bool imguiEnabled = true;
    
    auto* appDelegate = new VulkanApplicationDelegate{ instance, "Vulkan Application", 1600, 900, 2, DEBUG_OR_RELEASE(true, false), imguiEnabled };
    auto* application = new Application{ appDelegate };

    application->run();

    delete application;
    delete appDelegate;

    //system("pause");

    return 0;
}