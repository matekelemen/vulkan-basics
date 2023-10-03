#pragma once

// --- External Includes ---
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// --- Internal Includes ---
#include "VulkanInstance.hpp"

// --- STL Includes ---
#include <memory>


class WindowSurface
{
public:
    WindowSurface(const std::shared_ptr<VulkanInstance>& rp_vulkanInstance,
                  GLFWwindow* p_window)
        : _surface(),
          _p_window(p_window),
          _p_vulkanInstance(rp_vulkanInstance)
    {
        if (glfwCreateWindowSurface(_p_vulkanInstance->get(), _p_window, nullptr, &_surface) != VK_SUCCESS) {
            throw std::runtime_error("Window surface construction failed");
        }
    }

    ~WindowSurface()
    {
        vkDestroySurfaceKHR(_p_vulkanInstance->get(), _surface, nullptr);
    }

    ///@name Member Access
    ///@{

    const VkSurfaceKHR& get() const noexcept
    {
        return _surface;
    }

    GLFWwindow* getWindow() const noexcept
    {
        return _p_window;
    }

    ///@}

private:
    VkSurfaceKHR _surface;

    mutable GLFWwindow* _p_window; // <== GLFW has no concept of constness

    std::shared_ptr<VulkanInstance> _p_vulkanInstance;
}; // class WindowSurface
