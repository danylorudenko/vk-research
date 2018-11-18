#include "Device.hpp"
#include "Tools.hpp"

#include <iomanip>
#include <algorithm>
#include <memory>
#include <limits>

namespace VKW
{

Device::Device()
    : device_{ VK_NULL_HANDLE }
    , table_{ nullptr }
    , physicalDevice_{ VK_NULL_HANDLE }
    , physicalDeviceProperties_{}
{
}

Device::Device(DeviceDesc const& desc)
    : device_{ VK_NULL_HANDLE }
    , table_{ desc.table_ }
    , physicalDevice_{ VK_NULL_HANDLE }
    , physicalDeviceProperties_{}
{
    std::uint32_t physicalDeviceCount = 0;
    std::vector<VkPhysicalDevice> physicalDevices;
    {
        VK_ASSERT(table_->vkEnumeratePhysicalDevices(desc.instance_->Handle(), &physicalDeviceCount, nullptr));
        physicalDevices.resize(physicalDeviceCount);
        VK_ASSERT(table_->vkEnumeratePhysicalDevices(desc.instance_->Handle(), &physicalDeviceCount, physicalDevices.data()));
    }
    

    // Pick physical device
    {
        if (physicalDeviceCount == 0) {
            std::cerr << "FATAL: Error initializing VKW::Device (Vulkan instance couldn't find any physical devices in the system)" << std::endl;
            assert(physicalDeviceCount != 0);
        }


        auto validPhysicalDevices = std::vector<VkPhysicalDevice>{};
        auto deviceProperties = std::make_unique<PhysicalDeviceProperties>();


        for (auto i = 0u; i < physicalDeviceCount; ++i) {
            
            RequestDeviceProperties(physicalDevices[i], *deviceProperties);

            auto deviceValid = IsPhysicalDeviceValid(*deviceProperties, desc.requiredExtensions_);
            if (deviceValid) {
                validPhysicalDevices.emplace_back(physicalDevices[i]);
            }

            PrintPhysicalDeviceData(*deviceProperties);

            *deviceProperties = PhysicalDeviceProperties{};
        }

        if (validPhysicalDevices.size() == 1) {
            physicalDevice_ = physicalDevices[0];

        }
        else if (validPhysicalDevices.size() > 1) {
            for (auto i = 0u; i < validPhysicalDevices.size(); ++i) {

                RequestDeviceProperties(validPhysicalDevices[i], *deviceProperties);
                auto const isGPU = deviceProperties->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                *deviceProperties = PhysicalDeviceProperties{};
                
                if (isGPU) {
                    physicalDevice_ = validPhysicalDevices[i];
                    break;
                }
            }

            // if no discrete GPUs, choose any first
            if (physicalDevice_ == VK_NULL_HANDLE) {
                physicalDevice_ = validPhysicalDevices[0];
            }
        }



        if (physicalDevice_ != VK_NULL_HANDLE) {
            RequestDeviceProperties(physicalDevice_, physicalDeviceProperties_);
            std::cout << "CHOSEN DEVICE: " << physicalDeviceProperties_.properties.deviceName << std::endl;
        }
        else {
            std::cout << "FATAL: Error initializing VKW::Device (cannot find valid VkPhysicalDevice)" << std::endl;
            assert(false && "FATAL: Error initializing VKW::Device (cannot find valid VkPhysicalDevice)");
        }
    }


    // Create logical device
    {
        auto constexpr QUEUE_TYPE_COUNT = 3;
        VkFlags constexpr QUEUE_TYPE_FLAGS[QUEUE_TYPE_COUNT] = {
            VK_QUEUE_GRAPHICS_BIT,
            VK_QUEUE_COMPUTE_BIT,
            VK_QUEUE_TRANSFER_BIT
        };

        std::uint32_t const QUEUE_COUNTS[QUEUE_TYPE_COUNT] = {
            desc.graphicsPresentQueueCount_,
            desc.computeQueueCount_,
            desc.transferQueueCount_
        };
        
        auto constexpr INVALID_QUEUE_INDEX = std::numeric_limits<std::uint32_t>::max();
        auto const& queueFamilyProperties = physicalDeviceProperties_.queueFamilyProperties;
        auto const& presentationFamilies = physicalDeviceProperties_.presentationFamilies;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfoVec;
        std::vector<float*> queuePrioritiesVec;
        
        for(auto i = 0u; i < QUEUE_TYPE_COUNT; ++i) {
            if (QUEUE_COUNTS[i] == 0)
                continue;

            std::uint32_t chosenQueueFamily = INVALID_QUEUE_INDEX;
            for (auto j = 0u; j < queueFamilyProperties.size(); ++j) {
                bool const queueTypeGraphics = queueFamilyProperties[j].queueFlags & QUEUE_TYPE_FLAGS[0];

                bool const queueTypeSupported = queueFamilyProperties[j].queueFlags & QUEUE_TYPE_FLAGS[i];
                bool const queueCountSupported = queueFamilyProperties[j].queueCount >= QUEUE_COUNTS[i];
                bool const queuePresentSupported = std::find(presentationFamilies.cbegin(), presentationFamilies.cend(), j) != presentationFamilies.cend() ? true : false;

                if(queueTypeGraphics && !queuePresentSupported)
                    continue;


                if (queueTypeSupported && queueCountSupported) {
                    VkDeviceQueueCreateInfo queueCreateInfo;
                    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.pNext = nullptr;
                    queueCreateInfo.queueFamilyIndex = chosenQueueFamily = j;
                    queueCreateInfo.queueCount = QUEUE_COUNTS[i];
                    queueCreateInfo.flags = VK_FLAGS_NONE;
                    float* queuePriorities = new float[QUEUE_COUNTS[i]];
                    for (auto k = 0u; k < QUEUE_COUNTS[i]; ++k)
                        queuePriorities[k] = 1.0f;
                    queueCreateInfo.pQueuePriorities = queuePriorities;

                    queuePrioritiesVec.emplace_back(queuePriorities);
                    queueCreateInfoVec.emplace_back(queueCreateInfo);

                    // Save queues info for own usage
                    DeviceQueueFamilyInfo queueInfo;
                    queueInfo.familyIndex_ = chosenQueueFamily;
                    queueInfo.count_ = QUEUE_COUNTS[i];
                    queueInfo.presentationSupported_ = queuePresentSupported;
                    if (QUEUE_TYPE_FLAGS[i] & VK_QUEUE_GRAPHICS_BIT) {
                        queueInfo.type_ = DeviceQueueType::GRAPHICS_PRESENT;
                    }
                    else if (QUEUE_TYPE_FLAGS[i] & VK_QUEUE_COMPUTE_BIT) {
                        queueInfo.type_ = DeviceQueueType::COMPUTE;
                    }
                    else if (QUEUE_TYPE_FLAGS[i] & VK_QUEUE_TRANSFER_BIT) {
                        queueInfo.type_ = DeviceQueueType::TRANSFER;
                    }
                    queueInfo_.emplace_back(queueInfo);

                    break;
                }
            }

            if (QUEUE_COUNTS[i] > 0) {
                assert(chosenQueueFamily != INVALID_QUEUE_INDEX && "Couldn't create all required queues");
            }
        }

        std::vector<char const*> requiredExtensionsC_str{};
        std::transform(
            desc.requiredExtensions_.begin(), desc.requiredExtensions_.end(), 
            std::back_inserter(requiredExtensionsC_str), 
            [](auto const& string){ return string.c_str(); });

        VkDeviceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_FLAGS_NONE;
        createInfo.pEnabledFeatures = &physicalDeviceProperties_.features;
        createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfoVec.size());
        createInfo.pQueueCreateInfos = queueCreateInfoVec.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensionsC_str.size());
        createInfo.ppEnabledExtensionNames = requiredExtensionsC_str.data();

        VK_ASSERT(table_->vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_));

        for (auto const arr : queuePrioritiesVec) {
            delete[] arr;
        }

        table_->GetDeviceProcAddresses(device_);
    }

}

Device::Device(Device&& rhs)
    : device_{ VK_NULL_HANDLE }
    , table_{ nullptr }
    , physicalDevice_{ VK_NULL_HANDLE }
    , physicalDeviceProperties_{}
{
    operator=(std::move(rhs));
}

Device& Device::operator=(Device&& rhs)
{
    std::swap(device_, rhs.device_);
    std::swap(table_, rhs.table_);
    std::swap(physicalDevice_, rhs.physicalDevice_);
    std::swap(physicalDeviceProperties_, rhs.physicalDeviceProperties_);
    std::swap(queueInfo_, rhs.queueInfo_);

    return *this;
}

VkDevice Device::Handle() const
{
    return device_;
}

VkPhysicalDevice Device::PhysicalDeviceHandle() const
{
    return physicalDevice_;
}

Device::operator bool() const
{
    return device_ != VK_NULL_HANDLE;
}

void Device::PrintPhysicalDeviceFormatProperties(VkFormat format)
{
    VkFormatProperties fProps;
    table_->vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &fProps);
    
    bool r1 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    bool r2 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    bool r3 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT;
    bool r4 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT;
    bool r5 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT;
    bool r6 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT;
    bool r7 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
    bool r8 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    bool r9 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    bool r10 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    bool r12 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT;
    bool r13 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT;
    bool r14 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    bool r15 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    bool r16 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    bool r18 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG;
    bool r19 = fProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT;
    bool r20 = false;

    std::cout << "Not implemented!" << std::endl;
}

VKW::Device::PhysicalDeviceProperties const& Device::Properties() const
{
    return physicalDeviceProperties_;
}

std::uint32_t Device::QueueFamilyCount() const
{
    return static_cast<std::uint32_t>(queueInfo_.size());
}

VKW::DeviceQueueFamilyInfo const& Device::GetQueueFamily(std::uint32_t index) const
{
    return queueInfo_[index];
}

Device::~Device()
{
    if (device_) {
        VK_ASSERT(table_->vkDeviceWaitIdle(device_));
        table_->vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }
}

bool Device::IsPhysicalDeviceValid(
    VKW::Device::PhysicalDeviceProperties const& deviceProperties,
    std::vector<std::string> const& requiredExtensions)
{    
    bool supportsGraphics = false;
    bool supportsExtensions = true;
    bool supportsSurface = false;

    auto const& queueFamilyProperties = deviceProperties.queueFamilyProperties;
    for (auto i = 0u; i < queueFamilyProperties.size(); ++i) {
        auto& queueProps = queueFamilyProperties[i];
        if (queueProps.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            queueProps.queueCount > 0)
            supportsGraphics = true;
    }

    auto const& supportedExtensions = deviceProperties.extensionProperties;
    for (auto i = 0u; i < requiredExtensions.size(); ++i) {
        const auto& extName = requiredExtensions[i];
        auto result = std::find_if(supportedExtensions.cbegin(), supportedExtensions.cend(),
        [&extName](auto const& extensionProperties) {
            return extName == extensionProperties.extensionName;
        });

        if (result == supportedExtensions.cend()) {
            supportsExtensions = false;
        }
    }

    if (deviceProperties.presentationFamilies.size() > 0) {
        supportsSurface = true;
    }

    return supportsGraphics && supportsExtensions && supportsSurface;
}

void Device::RequestDeviceProperties(
    VkPhysicalDevice targetDevice,
    VKW::Device::PhysicalDeviceProperties& deviceProperties)
{
    table_->vkGetPhysicalDeviceProperties(targetDevice, &deviceProperties.properties);
    table_->vkGetPhysicalDeviceMemoryProperties(targetDevice, &deviceProperties.memoryProperties);
    table_->vkGetPhysicalDeviceFeatures(targetDevice, &deviceProperties.features);

    auto queuePropsCount = 0u;
    table_->vkGetPhysicalDeviceQueueFamilyProperties(targetDevice, &queuePropsCount, nullptr);
    deviceProperties.queueFamilyProperties.resize(queuePropsCount);
    table_->vkGetPhysicalDeviceQueueFamilyProperties(targetDevice, &queuePropsCount, deviceProperties.queueFamilyProperties.data());

#ifdef _WIN32
    deviceProperties.presentationFamilies.clear();
    for (auto i = 0u; i < queuePropsCount; ++i) {
        VkBool32 presentationSupport = table_->vkGetPhysicalDeviceWin32PresentationSupportKHR(targetDevice, i);
        if (presentationSupport == VK_TRUE) {
            deviceProperties.presentationFamilies.push_back(i);
        }
    }
#endif

    auto extensionPropsCount = 0u;
    VK_ASSERT(table_->vkEnumerateDeviceExtensionProperties(targetDevice, nullptr, &extensionPropsCount, nullptr));
    deviceProperties.extensionProperties.resize(extensionPropsCount);
    VK_ASSERT(table_->vkEnumerateDeviceExtensionProperties(targetDevice, nullptr, &extensionPropsCount, deviceProperties.extensionProperties.data()));
}

void Device::PrintPhysicalDeviceData(VKW::Device::PhysicalDeviceProperties const& deviceProperties)
{
    auto const& properties = deviceProperties.properties;

    std::cout << "PHYSICAL DEVICE NAME: " << properties.deviceName << std::endl << std::endl;
    std::cout << "\t" << "Vendor ID: " << properties.vendorID << std::endl;
    std::cout << "\t" << "Device ID: " << properties.deviceID << std::endl;
    std::cout << "\t" << "Type: " << properties.deviceType << std::endl;
    std::cout << "\t" << "Driver version: " << properties.driverVersion << std::endl;
    std::cout << "\t" << "API Version: " << properties.apiVersion << std::endl;

    std::cout << "\t" << "Device limits: " << std::endl;
    std::cout << "\t\t" << "maxImageDimension1D:                              " << properties.limits.maxImageDimension1D << std::endl;
    std::cout << "\t\t" << "maxImageDimension2D:                              " << properties.limits.maxImageDimension2D << std::endl;
    std::cout << "\t\t" << "maxImageDimension3D:                              " << properties.limits.maxImageDimension3D << std::endl;
    std::cout << "\t\t" << "maxImageDimensionCube:                            " << properties.limits.maxImageArrayLayers << std::endl;
    std::cout << "\t\t" << "maxImageArrayLayers:                              " << properties.limits.maxImageDimensionCube << std::endl;
    std::cout << "\t\t" << "maxTexelBufferElements:                           " << properties.limits.maxTexelBufferElements << std::endl;
    std::cout << "\t\t" << "maxUniformBufferRange:                            " << properties.limits.maxUniformBufferRange << std::endl;
    std::cout << "\t\t" << "maxStorageBufferRange:                            " << properties.limits.maxStorageBufferRange << std::endl;
    std::cout << "\t\t" << "maxPushConstantsSize:                             " << properties.limits.maxPushConstantsSize << std::endl;
    std::cout << "\t\t" << "maxMemoryAllocationCount:                         " << properties.limits.maxMemoryAllocationCount << std::endl;
    std::cout << "\t\t" << "maxSamplerAllocationCount:                        " << properties.limits.maxSamplerAllocationCount << std::endl;
    std::cout << "\t\t" << "bufferImageGranularity:                           " << properties.limits.bufferImageGranularity << std::endl;
    std::cout << "\t\t" << "sparseAddressSpaceSize:                           " << properties.limits.sparseAddressSpaceSize << std::endl;
    std::cout << "\t\t" << "maxBoundDescriptorSets:                           " << properties.limits.maxBoundDescriptorSets << std::endl;
    std::cout << "\t\t" << "maxPerStageDescriptorSamplers:                    " << properties.limits.maxPerStageDescriptorSamplers << std::endl;
    std::cout << "\t\t" << "maxPerStageDescriptorUniformBuffers:              " << properties.limits.maxPerStageDescriptorUniformBuffers << std::endl;
    std::cout << "\t\t" << "maxPerStageDescriptorStorageBuffers:              " << properties.limits.maxPerStageDescriptorStorageBuffers << std::endl;
    std::cout << "\t\t" << "maxPerStageDescriptorSampledImages:               " << properties.limits.maxPerStageDescriptorSampledImages << std::endl;
    std::cout << "\t\t" << "maxPerStageDescriptorStorageImages:               " << properties.limits.maxPerStageDescriptorStorageImages << std::endl;
    std::cout << "\t\t" << "maxPerStageDescriptorInputAttachments:            " << properties.limits.maxPerStageDescriptorInputAttachments << std::endl;
    std::cout << "\t\t" << "maxPerStageResources:                             " << properties.limits.maxPerStageResources << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetSamplers:                         " << properties.limits.maxDescriptorSetSamplers << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetUniformBuffers:                   " << properties.limits.maxDescriptorSetUniformBuffers << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetUniformBuffersDynamic:            " << properties.limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetStorageBuffers:                   " << properties.limits.maxDescriptorSetStorageBuffers << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetStorageBuffersDynamic:            " << properties.limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetSampledImages:                    " << properties.limits.maxDescriptorSetSampledImages << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetStorageImages:                    " << properties.limits.maxDescriptorSetStorageImages << std::endl;
    std::cout << "\t\t" << "maxDescriptorSetInputAttachments:                 " << properties.limits.maxDescriptorSetInputAttachments << std::endl;
    std::cout << "\t\t" << "maxVertexInputAttributes:                         " << properties.limits.maxVertexInputAttributes << std::endl;
    std::cout << "\t\t" << "maxVertexInputBindings:                           " << properties.limits.maxVertexInputBindings << std::endl;
    std::cout << "\t\t" << "maxVertexInputAttributeOffset:                    " << properties.limits.maxVertexInputAttributeOffset << std::endl;
    std::cout << "\t\t" << "maxVertexInputBindingStride:                      " << properties.limits.maxVertexInputBindingStride << std::endl;
    std::cout << "\t\t" << "maxVertexOutputComponents:                        " << properties.limits.maxVertexOutputComponents << std::endl;
    std::cout << "\t\t" << "maxTessellationGenerationLevel:                   " << properties.limits.maxTessellationGenerationLevel << std::endl;
    std::cout << "\t\t" << "maxTessellationPatchSize:                         " << properties.limits.maxTessellationPatchSize << std::endl;
    std::cout << "\t\t" << "maxTessellationControlPerVertexInputComponents:   " << properties.limits.maxTessellationControlPerVertexInputComponents << std::endl;
    std::cout << "\t\t" << "maxTessellationControlPerVertexOutputComponents:  " << properties.limits.maxTessellationControlPerVertexOutputComponents << std::endl;
    std::cout << "\t\t" << "maxTessellationControlPerPatchOutputComponents:   " << properties.limits.maxTessellationControlPerPatchOutputComponents << std::endl;
    std::cout << "\t\t" << "maxTessellationControlTotalOutputComponents:      " << properties.limits.maxTessellationControlTotalOutputComponents << std::endl;
    std::cout << "\t\t" << "maxTessellationEvaluationInputComponents:         " << properties.limits.maxTessellationEvaluationInputComponents << std::endl;
    std::cout << "\t\t" << "maxTessellationEvaluationOutputComponents:        " << properties.limits.maxTessellationEvaluationOutputComponents << std::endl;
    std::cout << "\t\t" << "maxGeometryShaderInvocations:                     " << properties.limits.maxGeometryShaderInvocations << std::endl;
    std::cout << "\t\t" << "maxGeometryInputComponents:                       " << properties.limits.maxGeometryInputComponents << std::endl;
    std::cout << "\t\t" << "maxGeometryOutputComponents:                      " << properties.limits.maxGeometryOutputComponents << std::endl;
    std::cout << "\t\t" << "maxGeometryOutputVertices:                        " << properties.limits.maxGeometryOutputVertices << std::endl;
    std::cout << "\t\t" << "maxGeometryTotalOutputComponents:                 " << properties.limits.maxGeometryTotalOutputComponents << std::endl;
    std::cout << "\t\t" << "maxFragmentInputComponents:                       " << properties.limits.maxFragmentInputComponents << std::endl;
    std::cout << "\t\t" << "maxFragmentOutputAttachments:                     " << properties.limits.maxFragmentOutputAttachments << std::endl;
    std::cout << "\t\t" << "maxFragmentDualSrcAttachments:                    " << properties.limits.maxFragmentDualSrcAttachments << std::endl;
    std::cout << "\t\t" << "maxFragmentCombinedOutputResources:               " << properties.limits.maxFragmentCombinedOutputResources << std::endl;
    std::cout << "\t\t" << "maxComputeSharedMemorySize:                       " << properties.limits.maxComputeSharedMemorySize << std::endl;
    //<< "\t\t" <<maxComputeWorkGroupCount[3]:                              be" << properties.limits.maxComputeWorkGroupCount[3]:;;
    std::cout << "\t\t" << "maxComputeWorkGroupInvocations:                   " << properties.limits.maxComputeWorkGroupInvocations << std::endl;
    //<< "\t\t" <<maxComputeWorkGroupSize[3]:                              be" << properties.limits.maxComputeWorkGroupSize[3]:;
    std::cout << "\t\t" << "subPixelPrecisionBits:                            " << properties.limits.subPixelPrecisionBits << std::endl;
    std::cout << "\t\t" << "subTexelPrecisionBits:                            " << properties.limits.subTexelPrecisionBits << std::endl;
    std::cout << "\t\t" << "mipmapPrecisionBits:                              " << properties.limits.mipmapPrecisionBits << std::endl;
    std::cout << "\t\t" << "maxDrawIndexedIndexValue:                         " << properties.limits.maxDrawIndexedIndexValue << std::endl;
    std::cout << "\t\t" << "maxDrawIndirectCount:                             " << properties.limits.maxDrawIndirectCount << std::endl;
    std::cout << "\t\t" << "maxSamplerLodBias:                                " << properties.limits.maxSamplerLodBias << std::endl;
    std::cout << "\t\t" << "maxSamplerAnisotropy:                             " << properties.limits.maxSamplerAnisotropy << std::endl;
    std::cout << "\t\t" << "maxViewports:                                     " << properties.limits.maxViewports << std::endl;
    std::cout << "\t\t" << "maxViewportDimensions[2]:                         " << properties.limits.maxViewportDimensions[0] << ", " << properties.limits.maxViewportDimensions[1] << std::endl;
    std::cout << "\t\t" << "viewportBoundsRange[2]:                           " << properties.limits.viewportBoundsRange[0] << ", " << properties.limits.viewportBoundsRange[1] << std::endl;
    std::cout << "\t\t" << "viewportSubPixelBits:                             " << properties.limits.viewportSubPixelBits << std::endl;
    std::cout << "\t\t" << "minMemoryMapAlignment:                            " << properties.limits.minMemoryMapAlignment << std::endl;
    std::cout << "\t\t" << "minTexelBufferOffsetAlignment:                    " << properties.limits.minTexelBufferOffsetAlignment << std::endl;
    std::cout << "\t\t" << "minUniformBufferOffsetAlignment:                  " << properties.limits.minUniformBufferOffsetAlignment << std::endl;
    std::cout << "\t\t" << "minStorageBufferOffsetAlignment:                  " << properties.limits.minStorageBufferOffsetAlignment << std::endl;
    std::cout << "\t\t" << "minTexelOffset:                                   " << properties.limits.minTexelOffset << std::endl;
    std::cout << "\t\t" << "maxTexelOffset:                                   " << properties.limits.maxTexelOffset << std::endl;
    std::cout << "\t\t" << "minTexelGatherOffset:                             " << properties.limits.minTexelGatherOffset << std::endl;
    std::cout << "\t\t" << "maxTexelGatherOffset:                             " << properties.limits.maxTexelGatherOffset << std::endl;
    std::cout << "\t\t" << "minInterpolationOffset:                           " << properties.limits.minInterpolationOffset << std::endl;
    std::cout << "\t\t" << "maxInterpolationOffset:                           " << properties.limits.maxInterpolationOffset << std::endl;
    std::cout << "\t\t" << "subPixelInterpolationOffsetBits:                  " << properties.limits.subPixelInterpolationOffsetBits << std::endl;
    std::cout << "\t\t" << "maxFramebufferWidth:                              " << properties.limits.maxFramebufferWidth << std::endl;
    std::cout << "\t\t" << "maxFramebufferHeight:                             " << properties.limits.maxFramebufferHeight << std::endl;
    std::cout << "\t\t" << "maxFramebufferLayers:                             " << properties.limits.maxFramebufferLayers << std::endl;
    std::cout << "\t\t" << "framebufferColorSampleCounts:                     " << properties.limits.framebufferColorSampleCounts << std::endl;
    std::cout << "\t\t" << "framebufferDepthSampleCounts:                     " << properties.limits.framebufferDepthSampleCounts << std::endl;
    std::cout << "\t\t" << "framebufferStencilSampleCounts:                   " << properties.limits.framebufferStencilSampleCounts << std::endl;
    std::cout << "\t\t" << "framebufferNoAttachmentsSampleCounts:             " << properties.limits.framebufferNoAttachmentsSampleCounts << std::endl;
    std::cout << "\t\t" << "maxColorAttachments:                              " << properties.limits.maxColorAttachments << std::endl;
    std::cout << "\t\t" << "sampledImageColorSampleCounts:                    " << properties.limits.sampledImageColorSampleCounts << std::endl;
    std::cout << "\t\t" << "sampledImageIntegerSampleCounts:                  " << properties.limits.sampledImageIntegerSampleCounts << std::endl;
    std::cout << "\t\t" << "sampledImageDepthSampleCounts:                    " << properties.limits.sampledImageDepthSampleCounts << std::endl;
    std::cout << "\t\t" << "sampledImageStencilSampleCounts:                  " << properties.limits.sampledImageStencilSampleCounts << std::endl;
    std::cout << "\t\t" << "storageImageSampleCounts:                         " << properties.limits.storageImageSampleCounts << std::endl;
    std::cout << "\t\t" << "maxSampleMaskWords:                               " << properties.limits.maxSampleMaskWords << std::endl;
    std::cout << "\t\t" << "timestampComputeAndGraphics:                      " << properties.limits.timestampComputeAndGraphics << std::endl;
    std::cout << "\t\t" << "timestampPeriod:                                  " << properties.limits.timestampPeriod << std::endl;
    std::cout << "\t\t" << "maxClipDistances:                                 " << properties.limits.maxClipDistances << std::endl;
    std::cout << "\t\t" << "maxCullDistances:                                 " << properties.limits.maxCullDistances << std::endl;
    std::cout << "\t\t" << "maxCombinedClipAndCullDistances:                  " << properties.limits.maxCombinedClipAndCullDistances << std::endl;
    std::cout << "\t\t" << "discreteQueuePriorities:                          " << properties.limits.discreteQueuePriorities << std::endl;
    std::cout << "\t\t" << "pointSizeRange[2]:                                " << properties.limits.pointSizeRange[0] << ", " << properties.limits.pointSizeRange[1] << std::endl;
    std::cout << "\t\t" << "lineWidthRange[2]:                                " << properties.limits.lineWidthRange[0] << ", " << properties.limits.lineWidthRange[1] << std::endl;
    std::cout << "\t\t" << "pointSizeGranularity:                             " << properties.limits.pointSizeGranularity << std::endl;
    std::cout << "\t\t" << "lineWidthGranularity:                             " << properties.limits.lineWidthGranularity << std::endl;
    std::cout << "\t\t" << "strictLines:                                      " << properties.limits.strictLines << std::endl;
    std::cout << "\t\t" << "standardSampleLocations:                          " << properties.limits.standardSampleLocations << std::endl;
    std::cout << "\t\t" << "optimalBufferCopyOffsetAlignment:                 " << properties.limits.optimalBufferCopyOffsetAlignment << std::endl;
    std::cout << "\t\t" << "optimalBufferCopyRowPitchAlignment:               " << properties.limits.optimalBufferCopyRowPitchAlignment << std::endl;
    std::cout << "\t\t" << "nonCoherentAtomSize:                              " << properties.limits.nonCoherentAtomSize << std::endl << std::endl;

    std::cout << "\t" << "Sparce Properties: " << std::endl;
    std::cout << "\t\t" << "residencyAlignedMipSize: " << properties.sparseProperties.residencyAlignedMipSize << std::endl;
    std::cout << "\t\t" << "residencyNonResidentStrict: " << properties.sparseProperties.residencyNonResidentStrict << std::endl;
    std::cout << "\t\t" << "residencyStandard2DBlockShape: " << properties.sparseProperties.residencyStandard2DBlockShape << std::endl;
    std::cout << "\t\t" << "residencyStandard2DMultisampleBlockShape: " << properties.sparseProperties.residencyStandard2DMultisampleBlockShape << std::endl;
    std::cout << "\t\t" << "residencyStandard3DBlockShape: " << properties.sparseProperties.residencyStandard3DBlockShape << std::endl << std::endl;


    auto const& memoryProperties = deviceProperties.memoryProperties; 
    
    std::cout << "\t" << "Memory Properties: " << std::endl;
    std::cout << "\t\t" << "memoryHeapCount: " << memoryProperties.memoryHeapCount << std::endl;
    for (auto i = 0u; i < memoryProperties.memoryHeapCount; ++i) {
        std::cout << "\t\t\tHeap " << i << ": " << std::endl;
        std::cout << "\t\t\t\tsize: " << memoryProperties.memoryHeaps[i].size << std::endl;
        std::cout << "\t\t\t\tflag VK_MEMORY_HEAP_DEVICE_LOCAL_BIT: " << static_cast<bool>(memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) << std::endl;
        std::cout << "\t\t\t\tflag VK_MEMORY_HEAP_MULTI_INSTANCE_BIT: " << static_cast<bool>(memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) << std::endl;
        std::cout << "\t\t\t\tflag VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR: " << static_cast<bool>(memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR) << std::endl << std::endl;
    }

    std::cout << "\t\t" << "memoryTypeCount: " << memoryProperties.memoryTypeCount << std::endl;
    for (auto i = 0u; i < memoryProperties.memoryTypeCount; ++i) {
        std::cout << "\t\t\tMemory type " << i << ": " << std::endl;
        std::cout << "\t\t\theap: " << memoryProperties.memoryTypes[i].heapIndex << std::endl;
        std::cout << "\t\t\tflags: " << std::endl;
        std::cout << "\t\t\t\tVK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: " << static_cast<bool>(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) << std::endl;
        std::cout << "\t\t\t\tVK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: " << static_cast<bool>(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) << std::endl;
        std::cout << "\t\t\t\tVK_MEMORY_PROPERTY_HOST_COHERENT_BIT: " << static_cast<bool>(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) << std::endl;
        std::cout << "\t\t\t\tVK_MEMORY_PROPERTY_HOST_CACHED_BIT: " << static_cast<bool>(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) << std::endl;
        std::cout << "\t\t\t\tVK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT: " << static_cast<bool>(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) << std::endl;
        std::cout << "\t\t\t\tVK_MEMORY_PROPERTY_PROTECTED_BIT: " << static_cast<bool>(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) << std::endl << std::endl;
    }


    auto const& queueFamilyProperties = deviceProperties.queueFamilyProperties;

    std::cout << "\tQueue Family Properties: " << std::endl;
    for (auto i = 0u; i < queueFamilyProperties.size(); ++i) {
        std::cout << "\t\tqueue properties " << i << ": " << std::endl;
        std::cout << "\t\t\tqueueCount: " << queueFamilyProperties[i].queueCount << std::endl;
        std::cout << "\t\t\tVK_QUEUE_GRAPHICS_BIT: " << static_cast<bool>(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) << std::endl;
        std::cout << "\t\t\tVK_QUEUE_COMPUTE_BIT: " << static_cast<bool>(queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) << std::endl;
        std::cout << "\t\t\tVK_QUEUE_TRANSFER_BIT: " << static_cast<bool>(queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) << std::endl;
        std::cout << "\t\t\tVK_QUEUE_SPARSE_BINDING_BIT: " << static_cast<bool>(queueFamilyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) << std::endl;
        std::cout << "\t\t\tVK_QUEUE_PROTECTED_BIT: " << static_cast<bool>(queueFamilyProperties[i].queueFlags & VK_QUEUE_PROTECTED_BIT) << std::endl << std::endl;
    }


    auto const& extensionProperties = deviceProperties.extensionProperties;

    std::cout << "\tDevice Extension Properties: " << std::endl;
    for (auto i = 0u; i < extensionProperties.size(); ++i) {
        std::cout << "\t\textensionProperties " << i << ": " << std::endl;
        std::cout << "\t\t\textensionName: " << extensionProperties[i].extensionName << std::endl;
        std::cout << "\t\t\tspecVersion: " << extensionProperties[i].specVersion << std::endl << std::endl;
        
    }


    auto const& features = deviceProperties.features;

    std::cout << "\tDevice Features:" << std::endl;
    std::cout << "\t\t" << "robustBufferAccess: " << features.robustBufferAccess << std::endl;
    std::cout << "\t\t" << "fullDrawIndexUint32: " << features.fullDrawIndexUint32 << std::endl;
    std::cout << "\t\t" << "imageCubeArray: " << features.imageCubeArray << std::endl;
    std::cout << "\t\t" << "independentBlend: " << features.independentBlend << std::endl;
    std::cout << "\t\t" << "geometryShader: " << features.geometryShader << std::endl;
    std::cout << "\t\t" << "tessellationShader: " << features.tessellationShader << std::endl;
    std::cout << "\t\t" << "sampleRateShading: " << features.sampleRateShading << std::endl;
    std::cout << "\t\t" << "dualSrcBlend: " << features.dualSrcBlend << std::endl;
    std::cout << "\t\t" << "logicOp: " << features.logicOp << std::endl;
    std::cout << "\t\t" << "multiDrawIndirect: " << features.multiDrawIndirect << std::endl;
    std::cout << "\t\t" << "drawIndirectFirstInstance: " << features.drawIndirectFirstInstance << std::endl;
    std::cout << "\t\t" << "depthClamp: " << features.depthClamp << std::endl;
    std::cout << "\t\t" << "depthBiasClamp: " << features.depthBiasClamp << std::endl;
    std::cout << "\t\t" << "fillModeNonSolid: " << features.fillModeNonSolid << std::endl;
    std::cout << "\t\t" << "depthBounds: " << features.depthBounds << std::endl;
    std::cout << "\t\t" << "wideLines: " << features.wideLines << std::endl;
    std::cout << "\t\t" << "largePoints: " << features.largePoints << std::endl;
    std::cout << "\t\t" << "alphaToOne: " << features.alphaToOne << std::endl;
    std::cout << "\t\t" << "multiViewport: " << features.multiViewport << std::endl;
    std::cout << "\t\t" << "samplerAnisotropy: " << features.samplerAnisotropy << std::endl;
    std::cout << "\t\t" << "textureCompressionETC2: " << features.textureCompressionETC2 << std::endl;
    std::cout << "\t\t" << "textureCompressionASTC_LDR: " << features.textureCompressionASTC_LDR << std::endl;
    std::cout << "\t\t" << "textureCompressionBC: " << features.textureCompressionBC << std::endl;
    std::cout << "\t\t" << "occlusionQueryPrecise: " << features.occlusionQueryPrecise << std::endl;
    std::cout << "\t\t" << "pipelineStatisticsQuery: " << features.pipelineStatisticsQuery << std::endl;
    std::cout << "\t\t" << "vertexPipelineStoresAndAtomics: " << features.vertexPipelineStoresAndAtomics << std::endl;
    std::cout << "\t\t" << "fragmentStoresAndAtomics: " << features.fragmentStoresAndAtomics << std::endl;
    std::cout << "\t\t" << "shaderTessellationAndGeometryPointSize: " << features.shaderTessellationAndGeometryPointSize << std::endl;
    std::cout << "\t\t" << "shaderImageGatherExtended: " << features.shaderImageGatherExtended << std::endl;
    std::cout << "\t\t" << "shaderStorageImageExtendedFormats: " << features.shaderStorageImageExtendedFormats << std::endl;
    std::cout << "\t\t" << "shaderStorageImageMultisample: " << features.shaderStorageImageMultisample << std::endl;
    std::cout << "\t\t" << "shaderStorageImageReadWithoutFormat: " << features.shaderStorageImageReadWithoutFormat << std::endl;
    std::cout << "\t\t" << "shaderStorageImageWriteWithoutFormat: " << features.shaderStorageImageWriteWithoutFormat << std::endl;
    std::cout << "\t\t" << "shaderUniformBufferArrayDynamicIndexing: " << features.shaderUniformBufferArrayDynamicIndexing << std::endl;
    std::cout << "\t\t" << "shaderSampledImageArrayDynamicIndexing: " << features.shaderSampledImageArrayDynamicIndexing << std::endl;
    std::cout << "\t\t" << "shaderStorageBufferArrayDynamicIndexing: " << features.shaderStorageBufferArrayDynamicIndexing << std::endl;
    std::cout << "\t\t" << "shaderStorageImageArrayDynamicIndexing: " << features.shaderStorageImageArrayDynamicIndexing << std::endl;
    std::cout << "\t\t" << "shaderClipDistance: " << features.shaderClipDistance << std::endl;
    std::cout << "\t\t" << "shaderCullDistance: " << features.shaderCullDistance << std::endl;
    std::cout << "\t\t" << "shaderFloat64: " << features.shaderFloat64 << std::endl;
    std::cout << "\t\t" << "shaderInt64: " << features.shaderInt64 << std::endl;
    std::cout << "\t\t" << "shaderInt16: " << features.shaderInt16 << std::endl;
    std::cout << "\t\t" << "shaderResourceResidency: " << features.shaderResourceResidency << std::endl;
    std::cout << "\t\t" << "shaderResourceMinLod: " << features.shaderResourceMinLod << std::endl;
    std::cout << "\t\t" << "sparseBinding: " << features.sparseBinding << std::endl;
    std::cout << "\t\t" << "sparseResidencyBuffer: " << features.sparseResidencyBuffer << std::endl;
    std::cout << "\t\t" << "sparseResidencyImage2D: " << features.sparseResidencyImage2D << std::endl;
    std::cout << "\t\t" << "sparseResidencyImage3D: " << features.sparseResidencyImage3D << std::endl;
    std::cout << "\t\t" << "sparseResidency2Samples: " << features.sparseResidency2Samples << std::endl;
    std::cout << "\t\t" << "sparseResidency4Samples: " << features.sparseResidency4Samples << std::endl;
    std::cout << "\t\t" << "sparseResidency8Samples: " << features.sparseResidency8Samples << std::endl;
    std::cout << "\t\t" << "sparseResidency16Samples: " << features.sparseResidency16Samples << std::endl;
    std::cout << "\t\t" << "sparseResidencyAliased: " << features.sparseResidencyAliased << std::endl;
    std::cout << "\t\t" << "variableMultisampleRate: " << features.variableMultisampleRate << std::endl;
    std::cout << "\t\t" << "inheritedQueries: " << features.inheritedQueries << std::endl << std::endl;
}

}