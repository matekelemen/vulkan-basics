#pragma once

// --- External Includes ---
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// --- Internal Includes ---
#include "utilities.hpp"

// --- STL Includes ---
#include <string>
#include <memory>


class Application
{
public:
    Application();

    ~Application();

    void run();

private:
    void initVulkan();

    void initWindow();

    void mainLoop();

    template <concepts::Iterator TOutputIt>
    void getRequiredExtensions(TOutputIt it);

    template <concepts::Iterator<std::string> TInputIt, concepts::Iterator TOutputIt>
    void getSupportedLayers(TInputIt begin, TInputIt end, TOutputIt it_out);

    void initDebugMessenger();

    void createVkInstance();

    void createSurface();

    void createPhysicalDevice();

    void createLogicalDevice();

    void createSwapChain();

    void createImageViews();

    template <concepts::Iterator TOutputIt>
    static void getExtensions(TOutputIt it);

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* p_data,
                                                        void* p_userData);
private:
    struct Impl;
    std::unique_ptr<Impl> _p_impl;
}; // class Application