#include <application\VulkanApplicationDelegate.hpp>

int main()
{
#if defined DEBUG || _DEBUG
    bool debug = true;
#else
    bool debug = false;
#endif


    HINSTANCE instance = GetModuleHandle(nullptr);
    
    bool imguiEnabled = true;
    
    auto* appDelegate = new VulkanApplicationDelegate{ instance, "Vulkan Application", 1600, 900, 2, debug, imguiEnabled };
    auto* application = new Application{ appDelegate };

    application->run();

    delete application;
    delete appDelegate;

    //system("pause");

    return 0;
}