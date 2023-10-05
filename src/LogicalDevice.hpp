#pragma once

// --- External Includes ---
#include "vulkan/vulkan.hpp"

#if defined(__APPLE__) && __APPLE__
#include "vulkan/vulkan_beta.h"
#endif

// --- Internal Includes ---
#include "utilities.hpp"
#include "PhysicalDevice.hpp"

// --- STL Includes ---
#include <unordered_set>
#include <span>


class LogicalDevice
{
public:
    LogicalDevice()
        : _device(VK_NULL_HANDLE),
          _p_physicalDevice()
    {
    }

    LogicalDevice(const std::shared_ptr<PhysicalDevice>& rp_physicalDevice)
        : LogicalDevice(rp_physicalDevice,
                        [](){
                            std::vector<PhysicalDevice::Feature> features;
                            LogicalDevice::getRequiredFeatures(std::back_inserter(features));
                            return features;
                        }(),
                        [](){
                            std::vector<const char*> extensions;
                            LogicalDevice::getRequiredExtensions(std::back_inserter(extensions));
                            return extensions;
                        }())
    {
    }

    virtual ~LogicalDevice()
    {
        if (_device != VK_NULL_HANDLE) {
            vkDestroyDevice(_device, nullptr);
        }
    }

    ///@name Queries
    ///@{

    /// @tparam TIterator output iterator with @ref PhysicalDevice::Feature as value type.
    /// @return the output iterator pointing to the new end of the modified container.
    template <class TIterator>
    static TIterator getRequiredFeatures(TIterator it_output)
    {
        return it_output;
    }

    /// @tparam TIterator output iterator with @a const @a char* as value type.
    /// @return the output iterator pointing to the new end of the modified container.
    template <class TIterator>
    static TIterator getRequiredExtensions(TIterator it_output)
    {
        return it_output;
    }

    ///@}
    ///@name Member Access
    ///@{

    const VkDevice& getDevice() const
    {
        return _device;
    }

    const PhysicalDevice& getPhysicalDevice() const
    {
        return *_p_physicalDevice;
    }

    ///@}
    ///@name Queries
    ///@{

    VkQueue getQueue() const
    {
        auto queueFamily = _p_physicalDevice->getQueueFamily({}); // @todo
        VkQueue queue;
        vkGetDeviceQueue(_device, queueFamily.graphics.value(), 0, &queue);
        return queue;
    }

    ///@}
protected:
    LogicalDevice(const std::shared_ptr<PhysicalDevice>& rp_physicalDevice,
                  const std::vector<PhysicalDevice::Feature>& r_requiredFeatures,
                  const std::vector<const char*>& r_requiredExtensions)
        : LogicalDevice(rp_physicalDevice,
                        {r_requiredFeatures.data(), r_requiredFeatures.size()},
                        {r_requiredExtensions.data(), r_requiredExtensions.size()})
    {
    }

    LogicalDevice(const std::shared_ptr<PhysicalDevice>& rp_physicalDevice,
                  std::span<const PhysicalDevice::Feature> requiredFeatures,
                  std::span<const char* const> requiredExtensions)
        : _device(VK_NULL_HANDLE),
          _p_physicalDevice(rp_physicalDevice)
    {
        const auto queueFamily = rp_physicalDevice->getQueueFamily({});
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        // Collect unique queue families
        std::unordered_set<uint32_t> uniqueQueueFamilies;
        if (queueFamily.graphics.has_value())
            uniqueQueueFamilies.insert(queueFamily.graphics.value());
        if (queueFamily.presentation.has_value())
            uniqueQueueFamilies.insert(queueFamily.presentation.value());

        for (auto family : uniqueQueueFamilies) {
            queueCreateInfos.push_back({});
            auto& r_createInfo = queueCreateInfos.back();
            r_createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            r_createInfo.queueFamilyIndex = queueFamily.graphics.value();
            r_createInfo.queueCount = 1;
            const float queuePriority = 1.0f;
            r_createInfo.pQueuePriorities = &queuePriority;
        }

        const auto features = PhysicalDevice::makeFeatures({requiredFeatures.data(), requiredFeatures.size()});

        VkDeviceCreateInfo createInfo {};
        if (!queueCreateInfos.empty()) {
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pEnabledFeatures = &features;
            createInfo.enabledExtensionCount = requiredExtensions.size();
            createInfo.ppEnabledExtensionNames = requiredExtensions.data();

            #if defined(__APPLE__) && __APPLE__
            //createInfo.flags = VK_KHR_portability_subset; // <== @todo apparently, I'll need VK_KHR_portability_subset but I've no idea where
            #endif

            // For legacy vulkan implementations, it would be necessary
            // to define the validation layers for the device separately,
            // but I'll just ignore that here.
        } else {
            throw std::runtime_error("No queue families found");
        }

        // Create the logical device
        if (vkCreateDevice(rp_physicalDevice->getDevice(), &createInfo, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("Logical device creation failed");
        }

        // Get its queue
        for (auto family : uniqueQueueFamilies) {
            _queues.push_back({});
            vkGetDeviceQueue(_device, family, 0, &_queues.back());
        }
    }

private:
    VkDevice _device;

    std::vector<VkQueue> _queues;

    std::shared_ptr<PhysicalDevice> _p_physicalDevice;
}; // class LogicalDevice



/// @brief Logical device meant specifically for graphics.
/// @details Requires @ref SwapChain support.
class GraphicsLogicalDevice : public LogicalDevice
{
public:
    GraphicsLogicalDevice()
    {
    }

    GraphicsLogicalDevice(const std::shared_ptr<PhysicalDevice>& rp_physicalDevice)
        : LogicalDevice(rp_physicalDevice,
                        [](){
                            std::vector<PhysicalDevice::Feature> features;
                            GraphicsLogicalDevice::getRequiredFeatures(std::back_inserter(features));
                            return features;
                        }(),
                        [](){
                            std::vector<const char*> extensions;
                            GraphicsLogicalDevice::getRequiredExtensions(std::back_inserter(extensions));
                            return extensions;
                        }())
    {
    }

    ///@name Queries
    ///@{

    /// @tparam TIterator output iterator with @ref PhysicalDevice::Feature as value type.
    /// @return the output iterator pointing to the new end of the modified container.
    template <class TIterator>
    static TIterator getRequiredFeatures(TIterator it_output)
    {
        return LogicalDevice::getRequiredFeatures(it_output);
    }

    /// @tparam TIterator output iterator with @a const @a char* as value type.
    /// @return the output iterator pointing to the new end of the modified container.
    template <class TIterator>
    static TIterator getRequiredExtensions(TIterator it_output)
    {
        it_output = LogicalDevice::getRequiredExtensions(it_output);
        *it_output++ = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        return it_output;
    }

    ///@}

protected:
    GraphicsLogicalDevice(const std::shared_ptr<PhysicalDevice>& rp_physicalDevice,
                          const std::vector<PhysicalDevice::Feature>& r_requiredFeatures,
                          const std::vector<const char*>& r_requiredExtensions)
        : LogicalDevice(rp_physicalDevice, r_requiredFeatures, r_requiredExtensions)
    {
    }

    GraphicsLogicalDevice(const std::shared_ptr<PhysicalDevice>& rp_physicalDevice,
                          std::span<const PhysicalDevice::Feature> requiredFeatures,
                          std::span<const char* const> requiredExtensions)
        : LogicalDevice(rp_physicalDevice, requiredFeatures, requiredExtensions)
    {
    }
}; // class GraphicsLogicalDevice
