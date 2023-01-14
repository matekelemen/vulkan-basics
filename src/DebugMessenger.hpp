#pragma once

// --- External Includes ---
#include "vulkan/vulkan.hpp"

// --- Internal Includes ---
#include "VulkanInstance.hpp"

// --- STL Includes ---
#include <memory>
#include <optional>


class DebugMessenger
{
public:
    DebugMessenger(const std::shared_ptr<VulkanInstance>& rp_vulkanInstance,
                   const VkDebugUtilsMessengerCreateInfoEXT& p_constructProperties,
                   const std::optional<VkAllocationCallbacks>& r_allocator = {});

    DebugMessenger(const DebugMessenger&) = delete;

    ~DebugMessenger();

    VkDebugUtilsMessengerEXT& get();

    const VkDebugUtilsMessengerEXT& get() const;

private:
    std::shared_ptr<VulkanInstance> _p_vulkanInstance;

    std::optional<VkAllocationCallbacks> _allocator;

    VkDebugUtilsMessengerEXT _messenger;
}; // class DebugMessenger
