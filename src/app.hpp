#pragma once

// --- External Includes ---
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// --- STL Includes ---
#include <type_traits>
#include <string>


namespace concepts {
template <class TIt, class TValue = void>
concept Iterator
= !std::is_same_v<typename std::iterator_traits<TIt>::iterator_category, void>
  && (std::is_same_v<TValue,void> || std::is_same_v<TValue,typename std::iterator_traits<TIt>::value_type>);
} // namespace concepts


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

    template <concepts::Iterator TOutputIt>
    static void getExtensions(TOutputIt it);

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* p_data,
                                                        void* p_userData);
private:
    GLFWwindow* _p_window;

    VkInstance _vulkanInstance;

    #ifndef NDEBUG
    VkDebugUtilsMessengerEXT _debugMessenger;

    static constexpr bool _enableValidationLayers = true;
    #else
    static constexpr bool _enableValidationLayers = false;
    #endif

    const unsigned _windowWidth = 800;

    const unsigned _windowHeight = 600;
}; // class Application