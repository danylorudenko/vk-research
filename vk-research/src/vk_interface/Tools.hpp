#pragma once

#include <iostream>

#define VK_FLAGS_NONE 0

#define VK_ASSERT(result)                           \
{                                                   \
    VkResult r = result;                            \
    if (result != VK_SUCCESS) {                     \
        std::cerr << "Fatal: VkResult is " << static_cast<int>(r) << std::endl << "FILE: " << __FILE__ << ", line: " << __LINE__ << std::endl; \
        assert(r == VK_SUCCESS);                    \
    }                                               \
}