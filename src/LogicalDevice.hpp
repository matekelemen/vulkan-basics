#pragma once

// --- External Includes ---
#include "vulkan/vulkan.hpp"

// --- Internal Includes ---
#include "utilities.hpp"
#include "PhysicalDevice.hpp"

// --- STL Includes ---
#include <unordered_set>


class LogicalDevice
{
public:
    LogicalDevice()
        : _device(VK_NULL_HANDLE),
          _p_physicalDevice()
    {
    }

    LogicalDevice(const std::shared_ptr<PhysicalDevice>& rp_physicalDevice)
        : _device(VK_NULL_HANDLE),
          _p_physicalDevice(rp_physicalDevice)
    {
        const auto queueFamily = rp_physicalDevice->getQueueFamily({}); // @todo
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

        auto features = this->getRequiredFeatures();

        VkDeviceCreateInfo createInfo {};
        if (!queueCreateInfos.empty()) {
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pEnabledFeatures = &features;
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

    ~LogicalDevice()
    {
        if (_device != VK_NULL_HANDLE) {
            vkDestroyDevice(_device, nullptr);
        }
    }

    ///@name Queries
    ///@{

    virtual VkPhysicalDeviceFeatures getRequiredFeatures() const
    {
        VkPhysicalDeviceFeatures deviceFeatures {};
        return deviceFeatures;
    }

    ///@}
    ///@name Member Access
    ///@{

    const VkDevice& getDevice() const
    {
        return _device;
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

private:
    VkDevice _device;

    std::vector<VkQueue> _queues;

    std::shared_ptr<PhysicalDevice> _p_physicalDevice;
}; // class LogicalDevice
