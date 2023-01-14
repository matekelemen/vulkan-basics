#pragma once

// --- External Includes ---
#include "vulkan/vulkan.hpp"

// --- Internal Includes ---
#include "utilities.hpp"
#include "PhysicalDevice.hpp"


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
        const auto queueFamily = rp_physicalDevice->getQueueFamily();
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily.graphics.value();
        queueCreateInfo.queueCount = 1;
        const float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        auto features = this->getRequiredFeatures();

        VkDeviceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;

        createInfo.pEnabledFeatures = &features;
        // For legacy vulkan implementations, it would be necessary
        // to define the validation layers for the device separately,
        // but I'll just ignore that here.

        if (vkCreateDevice(rp_physicalDevice->getDevice(), &createInfo, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("Logical device creation failed");
        }
    }

    ~LogicalDevice()
    {
        if (_device != VK_NULL_HANDLE) {
            vkDestroyDevice(_device, nullptr);
        }
    }

    virtual VkPhysicalDeviceFeatures getRequiredFeatures() const
    {
        VkPhysicalDeviceFeatures deviceFeatures {};
        return deviceFeatures;
    }

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
        auto queueFamily = _p_physicalDevice->getQueueFamily();
        VkQueue queue;
        vkGetDeviceQueue(_device, queueFamily.graphics.value(), 0, &queue);
        return queue;
    }

    ///@}

private:
    VkDevice _device;

    std::shared_ptr<PhysicalDevice> _p_physicalDevice;
}; // class LogicalDevice
