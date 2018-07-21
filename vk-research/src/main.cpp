#include "application\VulkanApplicationDelegate.hpp"

int main()
{
#if defined DEBUG || _DEBUG
    bool debug = true;
#else
    bool debug = false;
#endif


    HINSTANCE instance = GetModuleHandle(nullptr);
    
    auto* appDelegate = new VulkanApplicationDelegate{ instance, "Vulkan Application", 800, 600, debug };
    auto* application = new Application{ appDelegate };

    application->run();

    delete application;
    delete appDelegate;

    system("pause");

    return 0;
}