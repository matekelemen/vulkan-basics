// --- Internal Includes ---
#include "DebugMessenger.hpp"


DebugMessenger::DebugMessenger(const std::shared_ptr<VulkanInstance>& rp_vulkanInstance,
                               const VkDebugUtilsMessengerCreateInfoEXT& r_constructProperties,
                               const std::optional<VkAllocationCallbacks>& r_allocator)
    : _p_vulkanInstance(rp_vulkanInstance),
      _allocator(r_allocator)
{
    auto function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_p_vulkanInstance->get(), "vkCreateDebugUtilsMessengerEXT");
    if (function != nullptr) {
        const VkAllocationCallbacks* p_allocator = _allocator.has_value() ? &_allocator.value() : nullptr;
        function(_p_vulkanInstance->get(), &r_constructProperties, p_allocator, &_messenger);
    } else {
        throw std::runtime_error("Missing Vulkan extension: DebugUtilsMessenger");
    }
}


DebugMessenger::~DebugMessenger()
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_p_vulkanInstance->get(), "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        const VkAllocationCallbacks* p_allocator = _allocator.has_value() ? &_allocator.value() : nullptr;
        func(_p_vulkanInstance->get(), _messenger, p_allocator);
    }
}
